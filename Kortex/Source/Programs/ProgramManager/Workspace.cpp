#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include <Kortex/Events.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include "KAux.h"

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
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			GetDisplayModelOption().SaveDataViewLayout(m_ViewModel->GetView());
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_ViewModel = new DisplayModel();
		m_ViewModel->Create(this, m_MainSizer);
		GetDisplayModelOption().LoadDataViewLayout(m_ViewModel->GetView());

		IEvent::Bind(Events::VirtualFileSystemToggled, &Workspace::OnVFSToggled, this);
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

	void Workspace::OnVFSToggled(VirtualFileSystemEvent& event)
	{
		ScheduleReload();
	}

	wxString Workspace::GetID() const
	{
		return "KProgramManagerWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("ProgramManager.Name");
	}
	wxString Workspace::GetNameShort() const
	{
		return KTr("ProgramManager.NameShort");
	}
}
