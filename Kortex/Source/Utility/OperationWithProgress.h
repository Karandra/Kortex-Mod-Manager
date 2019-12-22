#pragma once
#include "stdafx.h"
#include "QuickThread.h"
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxWithOptions.h>

// TODO: These threading classes are inherently broken, try not to use them. Rewrite as soon as possible.
// Consider use of 'KxThread' class.
namespace Kortex::Utility
{
	class OperationWithProgressBase: public wxClientDataContainer
	{
		public:
			using NotifyEventTag = wxEventTypeTag<wxNotifyEvent>;
			using HandlerType = std::function<void()>;

		private:
			wxEvtHandler m_EventHandler;
			QuickThread* m_Thread = nullptr;
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
			OperationWithProgressBase(bool allowCancel);
			OperationWithProgressBase(HandlerType entryPoint, bool allowCancel);
			virtual ~OperationWithProgressBase();

		public:
			wxEvtHandler* GetEventHandler()
			{
				return &m_EventHandler;
			}

			void OnRun(HandlerType func)
			{
				m_ThreadEntry = std::move(func);
			}
			void OnEnd(HandlerType func)
			{
				m_OnEndHandler = func;
			}
			void OnCancel(HandlerType func)
			{
				m_OnCancelHandler = func;
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
}

namespace Kortex::Utility
{
	enum class OperationWithProgressDialogOptions
	{
		None = 1 << 0,
		RunOnEndBeforeDialogDestruction = 1 << 0,
	};

	class OperationWithProgressDialogBase:
		public OperationWithProgressBase,
		public KxWithOptions<OperationWithProgressDialogOptions, OperationWithProgressDialogOptions::None>
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
			OperationWithProgressDialogBase(bool allowCancel, wxWindow* window = nullptr);
			OperationWithProgressDialogBase(const HandlerType& entryPoint, bool allowCancel, wxWindow* window = nullptr);
			virtual ~OperationWithProgressDialogBase();

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

	template<class TEvent = KxFileOperationEvent>
	class OperationWithProgressDialog: public OperationWithProgressDialogBase
	{
		public:
			using EventClass = TEvent;
			using EventTypeTag = wxEventTypeTag<EventClass>;

		public:
			OperationWithProgressDialog(bool allowCancel, wxWindow* window = nullptr)
				:OperationWithProgressDialogBase(allowCancel, window)
			{
			}
			OperationWithProgressDialog(const HandlerType& entryPoint, bool allowCancel, wxWindow* window = nullptr)
				:OperationWithProgressDialogBase(entryPoint, allowCancel, window)
			{
			}

		public:
			void AddEvent(EventTypeTag type)
			{
				GetEventHandler()->Bind(type, &OperationWithProgressDialog::OnFileOperation, this);
			}
			virtual void LinkHandler(wxEvtHandler* eventHandler, wxEventType type) override
			{
				OperationWithProgressDialogBase::LinkHandler(eventHandler, type);
				AddEvent(static_cast<EventTypeTag>(type));
			}
	};
}