#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include "VirtualFileSystem/VirtualFSEvent.h"
#include "GameMods/ModManager/Workspace.h"

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;
	using namespace Kortex::ProgramManager;

	auto GetDisplayModelOption()
	{
		return GetAInstanceOptionOf<IProgramManager>(OName::Workspace, OName::DisplayModel);
	}
}

namespace Kortex::ProgramManager
{
	void Workspace::OnMainFSToggled(VirtualFSEvent& event)
	{
		ScheduleReload();
	}

	bool Workspace::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		m_ViewModel = new DisplayModel();
		m_ViewModel->Create(this, m_MainSizer);
		
		SetSizer(m_MainSizer);
		GetDisplayModelOption().LoadDataViewLayout(m_ViewModel->GetView());

		m_BroadcastReciever.Bind(VirtualFSEvent::EvtMainToggled, &Workspace::OnMainFSToggled, this);
		return true;
	}
	bool Workspace::OnOpenWorkspace()
	{
		size_t sel = m_ViewModel->GetRow(m_ViewModel->GetView()->GetSelection());
		m_ViewModel->RefreshItems();
		m_ViewModel->SelectItem(sel);
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		m_ViewModel->RefreshItems();
	}

	Workspace::~Workspace()
	{
		if (IsCreated())
		{
			GetDisplayModelOption().SaveDataViewLayout(m_ViewModel->GetView());
		}
	}

	wxString Workspace::GetID() const
	{
		return "KProgramManagerWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("ProgramManager.NameShort");
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
