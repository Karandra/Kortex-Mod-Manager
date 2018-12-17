#include "stdafx.h"
#include "KNewModDialog.h"
#include <Kortex/ModManager.hpp>
#include "Events/LogEvent.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxFile.h>

using namespace Kortex;
using namespace Kortex::ModManager;

void KNewModDialog::OnOK(wxNotifyEvent& event)
{
	if (event.GetId() == KxID_OK)
	{
		wxString name = GetValue();
		if (!name.IsEmpty())
		{
			const IGameMod* existingMod = IModManager::GetInstance()->FindModByID(name);
			if (existingMod)
			{
				Kortex::LogEvent(KTr("ModManager.NewMod.NameCollision"), LogLevel::Warning, this);
			}
			else
			{
				m_Name = name;
				event.Skip();
				return;
			}
		}
		else
		{
			Kortex::LogEvent(KTr("ModManager.NewMod.NameInvalid"), LogLevel::Warning, this);
		}
		event.Veto();
	}
	else
	{
		event.Skip();
	}
}

KNewModDialog::KNewModDialog(wxWindow* parent)
{
	if (Create(parent, KxID_NONE, KTr("ModManager.NewMod.DialogCaption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL))
	{
		SetLabel(KTr("ModManager.NewMod.DialogMessage"));
		GetDialogMainCtrl()->SetFocus();

		Bind(KxEVT_STDDIALOG_BUTTON, &KNewModDialog::OnOK, this);
		Center();
	}
}
KNewModDialog::~KNewModDialog()
{
}

//////////////////////////////////////////////////////////////////////////
void KNewModDialogEx::OnSelectFolder(wxNotifyEvent& event)
{
	if (event.GetId() == KxID_SELECT_FOLDER)
	{
		KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN_FOLDER);
		dialog.SetOptionEnabled(KxFBD_FORCE_FILE_SYSTEM, true);
		if (dialog.ShowModal() == KxID_OK)
		{
			m_FolderPath = dialog.GetResult();
			if (GetFolderName().IsEmpty())
			{
				SetValue(KxFile(m_FolderPath).GetFullName());
			}
		}

		UpdateLabelText();
		event.Veto();
	}
	else if (event.GetId() == KxID_OK)
	{
		if (m_FolderPath.IsEmpty())
		{
			Kortex::LogEvent(KTr("ModManager.NewMod.SelectedFolderNone"), LogLevel::Warning, this);
			event.Veto();
		}
		else
		{
			event.Skip();
		}
	}
	else
	{
		event.Skip();
	}
}
void KNewModDialogEx::OnChangeMethod(wxCommandEvent& event)
{
	m_IsLinkedMod = m_LinkedModCheckBox->GetValue();
	UpdateLabelText();
}

wxString KNewModDialogEx::GetMethodString(bool bLink) const
{
	if (bLink)
	{
		return KTr("ModManager.NewMod.FromFolderMessageLink");
	}
	else
	{
		return KTr("ModManager.NewMod.FromFolderMessageCopy");
	}
}
void KNewModDialogEx::UpdateLabelText()
{
	if (m_FolderPath.IsEmpty())
	{
		SetLabel(GetMethodString());
	}
	else
	{
		SetLabel(wxString::Format("%s\r\n%s: %s", GetMethodString(), KTr("ModManager.NewMod.SelectedFolder"), m_FolderPath));
	}
}

KNewModDialogEx::KNewModDialogEx(wxWindow* parent)
	:KNewModDialog(parent)
{
	SetCaption(KTr("ModManager.NewMod.DialogCaption"));
	SetLabel(GetMethodString(false));
	AddButton(KxID_SELECT_FOLDER, wxEmptyString, true);

	// Method checkbox
	m_LinkedModCheckBox = new wxCheckBox(GetContentWindow(), KxID_NONE, KTr("ModManager.NewMod.CreateAsLinkedMod"));
	GetContentWindowSizer()->Add(m_LinkedModCheckBox, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

	Bind(KxEVT_STDDIALOG_BUTTON, &KNewModDialogEx::OnSelectFolder, this);
	m_LinkedModCheckBox->Bind(wxEVT_CHECKBOX, &KNewModDialogEx::OnChangeMethod, this);

	AdjustWindow();
	Center();
}
KNewModDialogEx::~KNewModDialogEx()
{
}
