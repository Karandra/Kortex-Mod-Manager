#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include <Kortex/Events.hpp>
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
		return GetAInstanceOptionOf<IModManager>(OName::VirtualGameFolder, OName::UI, OName::DisplayModel);
	}
}

namespace Kortex::VirtualGameFolder
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		IEvent::Bind(Kortex::Events::ProfileSelected, &Workspace::OnViewInvalidated, this);
		IEvent::Bind(Kortex::Events::ModsReordered, &Workspace::OnViewInvalidated, this);
		IEvent::Bind(Kortex::Events::ModToggled, &Workspace::OnViewInvalidated, this);
		IEvent::Bind(Kortex::Events::ModFilesChanged, &Workspace::OnViewInvalidated, this);

		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			GetDisplayModelOption().SaveDataViewLayout(m_Model->GetView());
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_Model = new DisplayModel();
		m_Model->Create(this, m_MainSizer);
		m_Model->RefreshItems();

		m_SearchBox = new KxSearchBox(this, wxID_NONE);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &Workspace::OnModSerach, this);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &Workspace::OnModSerach, this);
		m_MainSizer->Add(m_SearchBox, 0, wxTOP|wxEXPAND, KLC_VERTICAL_SPACING);

		GetDisplayModelOption().LoadDataViewLayout(m_Model->GetView());
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
		m_Model->RefreshItems();
	}

	void Workspace::OnModSerach(wxCommandEvent& event)
	{
		if (m_Model->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
		{
			m_Model->RefreshItems();
		}
	}
	void Workspace::OnViewInvalidated(IEvent& event)
	{
		if (IsWorkspaceCreated())
		{
			ScheduleReload();
		}
	}

	wxString Workspace::GetID() const
	{
		return "KModManagerVirtualGameFolderWS";
	}
	wxString Workspace::GetName() const
	{
		return KTr("VirtualGameFolderWS.Name");
	}
	wxString Workspace::GetNameShort() const
	{
		return KTr("VirtualGameFolderWS.NameShort");
	}
}
