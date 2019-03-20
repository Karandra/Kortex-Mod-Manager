#include "stdafx.h"
#include "KOperationWithProgress.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxString.h>

void KOperationWithProgressBase::OnThreadEnd(wxNotifyEvent& event)
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
void KOperationWithProgressBase::OnProcessEvent(wxNotifyEvent& event)
{
	if (CanContinue())
	{
		m_Thread->QueueEvent(event);
	}
	else
	{
		event.Veto();
	}
}

void KOperationWithProgressBase::RunOnEnd()
{
	if (m_OnEndHandler)
	{
		m_OnEndHandler(this);
	}
}
void KOperationWithProgressBase::RunOnCancel()
{
	if (m_OnCancelHandler)
	{
		m_OnCancelHandler(this);
	}
}
void KOperationWithProgressBase::OnRunThread()
{
}
void KOperationWithProgressBase::Create()
{
	m_Thread = new KQuickThread([this](KQuickThread& thread)
	{
		m_ThreadEntry(this);
	}, &m_EventHandler);

	m_EventHandler.Bind(KEVT_QUICK_THREAD_END, &KOperationWithProgressBase::OnThreadEnd, this);
}

KOperationWithProgressBase::KOperationWithProgressBase(bool allowCancel)
	:m_AllowCancel(allowCancel)
{
}
KOperationWithProgressBase::KOperationWithProgressBase(const HandlerType& entryPoint, bool allowCancel)
	:m_ThreadEntry(entryPoint), m_AllowCancel(allowCancel)
{
}
KOperationWithProgressBase::~KOperationWithProgressBase()
{
}

void KOperationWithProgressBase::LinkHandler(wxEvtHandler* eventHandler, wxEventType type)
{
	eventHandler->Bind(static_cast<NotifyEventTag>(type), &KOperationWithProgressBase::OnProcessEvent, this);
}
void KOperationWithProgressBase::ProcessEvent(wxNotifyEvent& event)
{
	OnProcessEvent(event);
}

void KOperationWithProgressBase::Run()
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
bool KOperationWithProgressBase::Stop()
{
	wxCriticalSectionLocker lock(m_StopCS);
	if (m_Thread)
	{
		KQuickThread* thread = m_Thread;
		m_Thread = nullptr;
		return thread->Destroy();
	}
	return false;
}
bool KOperationWithProgressBase::CanContinue() const
{
	if (m_Thread)
	{
		return !m_Thread->TestDestroy();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void KOperationWithProgressDialogBase::OnDialogClose(wxCloseEvent& event)
{
	event.Veto();
	if (IsCancelAllowed())
	{
		Stop();
		RunOnCancel();
	}
}
void KOperationWithProgressDialogBase::OnDialogButton(wxNotifyEvent& event)
{
	event.Veto();
	if (IsCancelAllowed() && event.GetId() == KxID_CANCEL)
	{
		Stop();
		RunOnCancel();
	}
}

void KOperationWithProgressDialogBase::DestroyDialog()
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
void KOperationWithProgressDialogBase::SetParent(wxWindow* window)
{
	if (window)
	{
		m_ExactParent = window;
		m_Parent = wxGetTopLevelParent(window);
	}
}

void KOperationWithProgressDialogBase::Create()
{
	int buttons = IsCancelAllowed() ? KxBTN_CANCEL : KxBTN_NONE;
	m_ProgressDialog = new KxDualProgressDialog(m_Parent, KxID_NONE, m_Caption, wxDefaultPosition, wxDefaultSize, buttons);
	m_ProgressDialog->SetLabel(" ");
	m_ProgressDialog->GetPB1()->Pulse();
	m_ProgressDialog->GetPB2()->Pulse();

	m_ProgressDialog->Bind(KxEVT_STDDIALOG_BUTTON, &KOperationWithProgressDialogBase::OnDialogButton, this);
	m_ProgressDialog->Bind(wxEVT_CLOSE_WINDOW, &KOperationWithProgressDialogBase::OnDialogClose, this);

	KOperationWithProgressBase::Create();
}
void KOperationWithProgressDialogBase::RunOnEnd()
{
	if (IsOptionEnabled(KOWPD_OPTION_RUN_ONEND_BEFORE_DIALOG_DESTRUCTION))
	{
		KOperationWithProgressBase::RunOnEnd();
		DestroyDialog();
	}
	else
	{
		DestroyDialog();
		KOperationWithProgressBase::RunOnEnd();
	}
}
void KOperationWithProgressDialogBase::OnRunThread()
{
	if (m_Parent)
	{
		m_Parent->Disable();
	}
	m_ProgressDialog->Show();

	KOperationWithProgressBase::OnRunThread();
}
wxString KOperationWithProgressDialogBase::OnSetLabel(const wxString& label)
{
	KxDualProgressDialog* dialog = GetDialog();
	int width = GetDialog()->GetSize().GetWidth();
	int charWidth = dialog->GetCharWidth();

	return KxString::AbbreviateFilePath(label, (width / charWidth) - 5);
}

KOperationWithProgressDialogBase::KOperationWithProgressDialogBase(bool allowCancel, wxWindow* parent)
	:KOperationWithProgressBase(allowCancel)
{
	SetParent(parent);
}
KOperationWithProgressDialogBase::KOperationWithProgressDialogBase(const HandlerType& entryPoint, bool allowCancel, wxWindow* parent)
	:KOperationWithProgressBase(entryPoint, allowCancel)
{
	SetParent(parent);
}
KOperationWithProgressDialogBase::~KOperationWithProgressDialogBase()
{
	DestroyDialog();
}

void KOperationWithProgressDialogBase::OnFileOperation(KxFileOperationEvent& event)
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
