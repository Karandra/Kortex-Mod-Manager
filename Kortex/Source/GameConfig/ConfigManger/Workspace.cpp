#include "stdafx.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
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
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_View = new KxDataView2::View(this, KxID_NONE);

		m_MainSizer->Add(m_View, 1, wxEXPAND);
		return true;
	}

	bool Workspace::OnOpenWorkspace()
	{
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		KMainWindow::GetInstance()->ClearStatus(1);
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
	}

	wxString Workspace::GetID() const
	{
		return "ConfigManager::Workspace";
	}
	wxString Workspace::GetName() const
	{
		return GameConfigModule::GetInstance()->GetModuleInfo().GetName();
	}
	wxString Workspace::GetNameShort() const
	{
		return GameConfigModule::GetInstance()->GetModuleInfo().GetName();
	}
}
