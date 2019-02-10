#include "stdafx.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <Kortex/Utility.hpp>

namespace Kortex::GameConfig
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_DisplayModel.CreateView(this, m_MainSizer);
		m_DisplayModel.LoadView();
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
		m_DisplayModel.LoadView();
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
