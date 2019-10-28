#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include "GameMods/ModManager/Workspace.h"
#include <Kortex/ApplicationOptions.hpp>
#include <KxFramework/KxSearchBox.h>

namespace Kortex::Application::OName
{
	KortexDefOption(VirtualGameFolder);
}

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;

	auto GetDisplayModelOption()
	{
		return GetAInstanceOptionOf<IModManager>(OName::VirtualGameFolder, OName::DisplayModel);
	}
}

namespace Kortex::VirtualGameFolder
{
	void Workspace::OnModSerach(wxCommandEvent& event)
	{
		if (m_DisplayModel->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
		{
			m_DisplayModel->RefreshItems();
		}
	}
	void Workspace::OnViewInvalidated(BroadcastEvent& event)
	{
		ScheduleReload();
	}
	void Workspace::OnBeginReload(BroadcastEvent& event)
	{
		Disable();
		m_DisplayModel->ClearView();
	}
	void Workspace::OnEndReload(BroadcastEvent& event)
	{
		m_DisplayModel->RefreshItems();
		Enable();
	}

	bool Workspace::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		m_DisplayModel = new DisplayModel();
		m_DisplayModel->Create(this, m_MainSizer);
		m_DisplayModel->RefreshItems();

		m_SearchBox = new KxSearchBox(this, wxID_NONE);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &Workspace::OnModSerach, this);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &Workspace::OnModSerach, this);
		m_MainSizer->Add(m_SearchBox, 0, wxTOP|wxEXPAND, KLC_VERTICAL_SPACING);

		GetDisplayModelOption().LoadDataViewLayout(m_DisplayModel->GetView());

		// Events
		m_BroadcastReciever.Bind(ModEvent::EvtVirtualTreeInvalidated, &Workspace::OnViewInvalidated, this);
		m_BroadcastReciever.Bind(ModEvent::EvtBeginReload, &Workspace::OnBeginReload, this);
		m_BroadcastReciever.Bind(ModEvent::EvtEndReload, &Workspace::OnEndReload, this);

		return true;
	}
	bool Workspace::OnOpenWorkspace()
	{
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		m_DisplayModel->RefreshItems();
	}

	Workspace::~Workspace()
	{
		if (IsCreated())
		{
			GetDisplayModelOption().SaveDataViewLayout(m_DisplayModel->GetView());
		}
	}

	wxString Workspace::GetID() const
	{
		return "KModManagerVirtualGameFolderWS";
	}
	wxString Workspace::GetName() const
	{
		return KTr("VirtualGameFolderWS.NameShort");
	}
	IWorkspaceContainer* Workspace::GetPreferredContainer() const
	{
		IWorkspaceContainer* result = nullptr;
		IWorkspace::CallIfCreated<ModManager::Workspace>([&](ModManager::Workspace& workspace)
		{
			result = &workspace.GetWorkspaceContainer();
		});
		return result;
	}
}
