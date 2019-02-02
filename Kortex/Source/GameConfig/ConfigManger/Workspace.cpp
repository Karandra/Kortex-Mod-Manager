#include "stdafx.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow), m_Manager(*IGameConfigManager::GetInstance()), m_Translator(m_Manager.GetTranslator())
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
		m_View->AssignModel(new KxDataView2::Model(false));
		m_MainSizer->Add(m_View, 1, wxEXPAND);

		m_View->AppendColumn<KxDataView2::TextRenderer>(m_Translator.GetString("ConfigManager.View.Path"), ColumnID::Path);
		m_View->AppendColumn<KxDataView2::TextRenderer>(m_Translator.GetString("ConfigManager.View.Name"), ColumnID::Name);
		m_View->AppendColumn<KxDataView2::TextRenderer>(m_Translator.GetString("ConfigManager.View.Type"), ColumnID::Type);
		m_View->AppendColumn(m_Translator.GetString("ConfigManager.View.Value"), ColumnID::Value);

		OnReloadWorkspace();
		return true;
	}

	void Workspace::ClearView()
	{
		auto& items = m_View->GetRootNode().GetChildren();
		if (!items.empty())
		{
			for (size_t i = items.size() - 1; i != 0; --i)
			{
				items[i]->Detach();
			}
		}
	}
	void Workspace::LoadView()
	{
		ClearView();

		KxDataView2::Node& rootNode = m_View->GetRootNode();
		m_Manager.ForEachDefinition([&rootNode](const Definition& definition)
		{
			definition.ForEachGroup([&rootNode](ItemGroup& group)
			{
				rootNode.AttachChild(&group, rootNode.GetChildrenCount());

				group.ForEachItem([&group](Item& item)
				{
					group.AttachChild(&item, group.GetChildrenCount());
				});
			});
		});
		m_View->ItemsChanged();
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
		LoadView();
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
