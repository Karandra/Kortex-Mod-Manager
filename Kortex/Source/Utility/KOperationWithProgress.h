#pragma once
#include "stdafx.h"
#include "KQuickThread.h"
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxWithOptions.h>

class KOperationWithProgressBase: public wxClientDataContainer
{
	public:
		typedef wxEventTypeTag<wxNotifyEvent> NotifyEventTag;
		typedef std::function<void(KOperationWithProgressBase*)> HandlerType;

	private:
		wxEvtHandler m_EventHandler;
		KQuickThread* m_Thread = nullptr;
		bool m_AllowCancel = false;
		bool m_Ended = false;
		wxCriticalSection m_EndCS;
		wxCriticalSection m_StopCS;

		HandlerType m_ThreadEntry;
		HandlerType m_OnEndHandler;
		HandlerType m_OnCancelHandler;

	private:
		void OnThreadEnd(wxNotifyEvent& event);
		void OnProcessEvent(wxNotifyEvent& event);
		
	protected:
		virtual void RunOnEnd();
		virtual void RunOnCancel();
		virtual void OnRunThread();
		virtual void Create();

	public:
		KOperationWithProgressBase(bool allowCancel);
		KOperationWithProgressBase(const HandlerType& entryPoint, bool allowCancel);
		virtual ~KOperationWithProgressBase();

	public:
		wxEvtHandler* GetEventHandler()
		{
			return &m_EventHandler;
		}

		void OnRun(const HandlerType& f)
		{
			m_ThreadEntry = f;
		}
		void OnEnd(const HandlerType& f)
		{
			m_OnEndHandler = f;
		}
		void OnCancel(const HandlerType& f)
		{
			m_OnCancelHandler = f;
		}

		virtual void LinkHandler(wxEvtHandler* eventHandler, wxEventType type);
		virtual void ProcessEvent(wxNotifyEvent& event);

		virtual void Run();
		virtual bool Stop();
		bool CanContinue() const;
		bool IsCancelAllowed() const
		{
			return m_AllowCancel;
		}
};

//////////////////////////////////////////////////////////////////////////
enum KOperationWithProgressDialogOptions
{
	KOWPD_OPTION_NONE = 1 << 0,
	KOWPD_OPTION_RUN_ONEND_BEFORE_DIALOG_DESTRUCTION = 1 << 0,
};
class KOperationWithProgressDialogBase:
	public KOperationWithProgressBase,
	public KxWithOptions<KOperationWithProgressDialogOptions, KOWPD_OPTION_NONE>
{
	private:
		wxWindow* m_Parent = nullptr;
		wxWindow* m_ExactParent = nullptr;
		KxDualProgressDialog* m_ProgressDialog = nullptr;
		wxString m_Caption;

	private:
		void OnDialogClose(wxCloseEvent& event);
		void OnDialogButton(wxNotifyEvent& event);
		
		void DestroyDialog();
		void SetParent(wxWindow* window);

	protected:
		virtual void Create() override;
		virtual void RunOnEnd() override;
		virtual void OnRunThread() override;
		virtual wxString OnSetLabel(const wxString& label);

	public:
		KOperationWithProgressDialogBase(bool allowCancel, wxWindow* window = nullptr);
		KOperationWithProgressDialogBase(const HandlerType& entryPoint, bool allowCancel, wxWindow* window = nullptr);
		virtual ~KOperationWithProgressDialogBase();

	public:
		void OnFileOperation(KxFileOperationEvent& event);

		KxDualProgressDialog* GetDialog()
		{
			return m_ProgressDialog;
		}
		const wxString& GetDialogCaption() const
		{
			return m_Caption;
		}
		void SetDialogCaption(const wxString& caption)
		{
			m_Caption = caption;
		}

		wxWindow* GetParent() const
		{
			return m_Parent ? m_Parent : m_ExactParent;
		}
		wxWindow* GetExactParent() const
		{
			return m_ExactParent;
		}
};

//////////////////////////////////////////////////////////////////////////
template<class EventTagT = KxFileOperationEvent>
class KOperationWithProgressDialog: public KOperationWithProgressDialogBase
{
	public:
		using EventClass = EventTagT;
		using EventTypeTag = wxEventTypeTag<EventClass>;

	public:
		KOperationWithProgressDialog(bool allowCancel, wxWindow* window = nullptr)
			:KOperationWithProgressDialogBase(allowCancel, window)
		{
		}
		KOperationWithProgressDialog(const HandlerType& entryPoint, bool allowCancel, wxWindow* window = nullptr)
			:KOperationWithProgressDialogBase(entryPoint, allowCancel, window)
		{
		}

	public:
		void AddEvent(EventTypeTag type)
		{
			GetEventHandler()->Bind(type, &KOperationWithProgressDialog::OnFileOperation, this);
		}
		virtual void LinkHandler(wxEvtHandler* eventHandler, wxEventType type) override
		{
			KOperationWithProgressDialogBase::LinkHandler(eventHandler, type);
			AddEvent(static_cast<EventTypeTag>(type));
		}
};
