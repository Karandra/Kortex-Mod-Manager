#include "stdafx.h"
#include "Dialog.h"
#include <Kortex/Application.hpp>
#include "Application/Options/CmdLineDatabase.h"
#include "Utility/KAux.h"
#include "Network/ModNetwork/Nexus.h"
#include "Network/INetworkManager.h"

namespace Kortex::NetworkManager::NXMHandler
{
	bool Dialog::CreateUI(wxWindow* parent)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("NetworkManager.NXMHandler.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_CLOSE))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			SetInitialSize(wxSize(840, 470));
			SetDefaultButton(KxID_CLOSE);

			// Buttons
			m_UnregisterButton = AddButton(KxID_REMOVE, KTr("NetworkManager.NXMHandler.UnregisterAssociations"), true).GetControl();
			m_UnregisterButton->Bind(wxEVT_BUTTON, &Dialog::OnUnregisterAssociations, this);

			m_RegisterButton = AddButton(KxID_ADD, KTr("NetworkManager.NXMHandler.RegisterAssociations"), true).GetControl();
			m_RegisterButton->Bind(wxEVT_BUTTON, &Dialog::OnRegisterAssociations, this);

			// View
			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_Panel = new KxPanel(m_ContentPanel, KxID_NONE);
			m_Panel->SetSizer(sizer);

			m_DisplayModel = new DisplayModel(m_Options);
			m_DisplayModel->CreateView(m_Panel);
			sizer->Add(m_DisplayModel->GetView(), 1, wxEXPAND);

			m_RegisteredToLabel = new KxLabel(m_Panel, KxID_NONE, wxEmptyString);
			sizer->Add(m_RegisteredToLabel, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			PostCreate(wxDefaultPosition);
			return true;
		}
		return false;
	}
	void Dialog::UpdateButtons()
	{
		auto ResetLabel = [this]()
		{
			m_RegisteredToLabel->SetLabel(KTrf("NetworkManager.NXMHandler.RegisteredToLabel", KAux::MakeNoneLabel()));
		};

		if (m_NXMFileType)
		{
			const IApplication* app = IApplication::GetInstance();
			const bool isAssociated = m_FileTypeManager.IsAssociatedWith(m_NXMFileType, app->GetExecutablePath());

			m_RegisterButton->Enable(!isAssociated);
			m_UnregisterButton->Enable(isAssociated);

			if (wxString path = m_NXMFileType.GetOpenExecutable(); !path.IsEmpty())
			{
				m_RegisteredToLabel->SetLabel(KTrf("NetworkManager.NXMHandler.RegisteredToLabel", path));
			}
			else
			{
				ResetLabel();
			}
		}
		else
		{
			m_RegisterButton->Disable();
			m_UnregisterButton->Disable();
			ResetLabel();
		}
	}
	AppOption Dialog::GetOptions() const
	{
		return Application::GetGlobalOptionOf<INetworkManager>(NexusModNetwork::GetInstance()->GetName(), wxS("NXMHandlerDialog"));
	}

	void Dialog::OnRegisterAssociations(wxCommandEvent& event)
	{
		if (m_NXMFileType)
		{
			const IApplication* app = IApplication::GetInstance();

			KxFileTypeInfo info(m_NXMFileType.GetMimeType());
			info.SetIcon(app->GetExecutablePath());
			info.SetDescription("URL:NXM Protocol");
			info.SetShortDescription("nxm");
			info.AddExtension("nxm");
			info.SetOpenCommand(KxString::Format("\"%1\" -%2", app->GetExecutablePath(), CmdLineName::DownloadLink));

			m_NXMFileType = m_FileTypeManager.Associate(info);
			if (!m_NXMFileType)
			{
				BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, KTr("NetworkManager.NXMHandler.RegisterAssociationsFailed"), this);
			}
			UpdateButtons();
		}
	}
	void Dialog::OnUnregisterAssociations(wxCommandEvent& event)
	{
		if (m_NXMFileType)
		{
			if (!m_FileTypeManager.Unassociate(m_NXMFileType))
			{
				BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, KTr("NetworkManager.NXMHandler.UnregisterAssociationsFailed"), this);
			}
			UpdateButtons();
		}
	}

	Dialog::Dialog(wxWindow* parent, OptionStore& options)
		:m_Options(options)
	{
		CreateUI(parent);

		// Load file type info
		m_NXMFileType = m_FileTypeManager.FileTypeFromExtension("nxm");
		if (!m_NXMFileType)
		{
			BroadcastProcessor::Get().QueueEvent(LogEvent::EvtError, KTr("NetworkManager.NXMHandler.GetAssociationsFailed"), this);
		}

		AppOption uiOptions = GetOptions();
		uiOptions.LoadDataViewLayout(m_DisplayModel->GetView());
		uiOptions.LoadWindowGeometry(this);
		UpdateButtons();
	}
	Dialog::~Dialog()
	{
		AppOption uiOptions = GetOptions();
		uiOptions.SaveDataViewLayout(m_DisplayModel->GetView());
		uiOptions.SaveWindowGeometry(this);
	}
}
