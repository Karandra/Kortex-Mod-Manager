#include "stdafx.h"
#include "OperationWithProgress.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxString.h>

namespace Kortex::Utility
{
	void OperationWithProgressBase::OnThreadEnd(wxNotifyEvent& event)
	{
		wxCriticalSectionLocker lock(m_EndCS);
		if (!m_Ended)
		{
			m_Ended = true;

			m_Thread = nullptr;
			RunOnEnd();
			delete this;
		}
	}
	void OperationWithProgressBase::OnProcessEvent(wxNotifyEvent& event)
	{
		if (CanContinue())
		{
			m_Thread->QueueEvent(std::unique_ptr<wxEvent>(event.Clone()));
		}
		else
		{
			event.Veto();
		}
	}

	void OperationWithProgressBase::RunOnEnd()
	{
		if (m_OnEndHandler)
		{
			m_OnEndHandler();
		}
	}
	void OperationWithProgressBase::RunOnCancel()
	{
		if (m_OnCancelHandler)
		{
			m_OnCancelHandler();
		}
	}
	void OperationWithProgressBase::OnRunThread()
	{
	}
	void OperationWithProgressBase::Create()
	{
		m_Thread = new QuickThread(std::move(m_ThreadEntry), &m_EventHandler);
		m_EventHandler.Bind(QuickThread::EvtThreadEnd, &OperationWithProgressBase::OnThreadEnd, this);
	}

	OperationWithProgressBase::OperationWithProgressBase(bool allowCancel)
		:m_AllowCancel(allowCancel)
	{
	}
	OperationWithProgressBase::OperationWithProgressBase(HandlerType entryPoint, bool allowCancel)
		: m_ThreadEntry(std::move(entryPoint)), m_AllowCancel(allowCancel)
	{
	}
	OperationWithProgressBase::~OperationWithProgressBase()
	{
	}

	void OperationWithProgressBase::LinkHandler(wxEvtHandler* eventHandler, wxEventType type)
	{
		eventHandler->Bind(static_cast<NotifyEventTag>(type), &OperationWithProgressBase::OnProcessEvent, this);
	}
	void OperationWithProgressBase::ProcessEvent(wxNotifyEvent& event)
	{
		OnProcessEvent(event);
	}

	void OperationWithProgressBase::Run()
	{
		if (!m_Thread)
		{
			Create();
		}

		if (m_Thread)
		{
			OnRunThread();
			m_Thread->Run();
		}
	}
	bool OperationWithProgressBase::Stop()
	{
		wxCriticalSectionLocker lock(m_StopCS);
		if (m_Thread)
		{
			Utility::QuickThread* thread = m_Thread;
			m_Thread = nullptr;
			return thread->Destroy();
		}
		return false;
	}
	bool OperationWithProgressBase::CanContinue() const
	{
		if (m_Thread)
		{
			return !m_Thread->TestDestroy();
		}
		return false;
	}
}

namespace Kortex::Utility
{
	void OperationWithProgressDialogBase::OnDialogClose(wxCloseEvent& event)
	{
		event.Veto();
		if (IsCancelAllowed())
		{
			Stop();
			RunOnCancel();
		}
	}
	void OperationWithProgressDialogBase::OnDialogButton(wxNotifyEvent& event)
	{
		event.Veto();
		if (IsCancelAllowed() && event.GetId() == KxID_CANCEL)
		{
			Stop();
			RunOnCancel();
		}
	}

	void OperationWithProgressDialogBase::DestroyDialog()
	{
		if (m_ProgressDialog)
		{
			m_ProgressDialog->Destroy();
			m_ProgressDialog = nullptr;
		}

		if (m_Parent)
		{
			m_Parent->Enable();
			m_Parent->Update();
			m_Parent->UpdateWindowUI();
		}
		if (m_ExactParent)
		{
			m_ExactParent->SetFocus();
		}
	}
	void OperationWithProgressDialogBase::SetParent(wxWindow* window)
	{
		if (window)
		{
			m_ExactParent = window;
			m_Parent = wxGetTopLevelParent(window);
		}
	}

	void OperationWithProgressDialogBase::Create()
	{
		int buttons = IsCancelAllowed() ? KxBTN_CANCEL : KxBTN_NONE;
		m_ProgressDialog = new KxDualProgressDialog(m_Parent, KxID_NONE, m_Caption, wxDefaultPosition, wxDefaultSize, buttons);
		m_ProgressDialog->SetLabel(" ");
		m_ProgressDialog->GetPB1()->Pulse();
		m_ProgressDialog->GetPB2()->Pulse();

		m_ProgressDialog->Bind(KxEVT_STDDIALOG_BUTTON, &OperationWithProgressDialogBase::OnDialogButton, this);
		m_ProgressDialog->Bind(wxEVT_CLOSE_WINDOW, &OperationWithProgressDialogBase::OnDialogClose, this);

		OperationWithProgressBase::Create();
	}
	void OperationWithProgressDialogBase::RunOnEnd()
	{
		if (IsOptionEnabled(OperationWithProgressDialogOptions::RunOnEndBeforeDialogDestruction))
		{
			OperationWithProgressBase::RunOnEnd();
			DestroyDialog();
		}
		else
		{
			DestroyDialog();
			OperationWithProgressBase::RunOnEnd();
		}
	}
	void OperationWithProgressDialogBase::OnRunThread()
	{
		if (m_Parent)
		{
			m_Parent->Disable();
		}
		m_ProgressDialog->Show();

		OperationWithProgressBase::OnRunThread();
	}
	wxString OperationWithProgressDialogBase::OnSetLabel(const wxString& label)
	{
		KxDualProgressDialog* dialog = GetDialog();
		int width = GetDialog()->GetSize().GetWidth();
		int charWidth = dialog->GetCharWidth();

		return KxString::AbbreviateFilePath(label, (width / charWidth) - 5);
	}

	OperationWithProgressDialogBase::OperationWithProgressDialogBase(bool allowCancel, wxWindow* parent)
		:OperationWithProgressBase(allowCancel)
	{
		SetParent(parent);
	}
	OperationWithProgressDialogBase::OperationWithProgressDialogBase(const HandlerType& entryPoint, bool allowCancel, wxWindow* parent)
		: OperationWithProgressBase(entryPoint, allowCancel)
	{
		SetParent(parent);
	}
	OperationWithProgressDialogBase::~OperationWithProgressDialogBase()
	{
		DestroyDialog();
	}

	void OperationWithProgressDialogBase::OnFileOperation(KxFileOperationEvent& event)
	{
		KxDualProgressDialog* dialog = GetDialog();

		dialog->SetCaption(GetDialogCaption());
		dialog->SetLabel(OnSetLabel(event.GetCurrent()));
		if (event.GetEventType() != KxEVT_FILEOP_SEARCH)
		{
			int64_t nMinor = event.GetMinorProcessed();
			int64_t nMajor = event.GetMinorTotal();
			if (event.IsMinorKnown())
			{
				dialog->GetPB1()->SetValue(nMinor, nMajor);
			}
			else if (nMinor != -2 && nMajor != -2)
			{
				dialog->GetPB1()->Pulse();
			}

			if (event.IsMajorKnown())
			{
				dialog->GetPB2()->SetValue(event.GetMajorProcessed(), event.GetMajorTotal());
			}
			else
			{
				dialog->GetPB2()->Pulse();
			}
		}
		dialog->SetAutoSize(false);
	}
}