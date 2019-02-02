#include "stdafx.h"
#include "Workspace.h"
#include "Utility/KAux.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <Kortex/Utility.hpp>
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
			ClearView();
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
		m_Categories.clear();
	}
	void Workspace::LoadView()
	{
		ClearView();
		m_Manager.ForEachItem([this](Item& item)
		{
			KxDataView2::Node* parent = &m_View->GetRootNode();
			auto it = m_Categories.find(item.GetCategory());
			if (it != m_Categories.end())
			{
				parent = &it->second;
			}
			else
			{
				// Build category tree for this item
				wxString categoryPath;
				Utility::String::SplitBySeparator(item.GetCategory(), wxS('/'), [this, &parent, &categoryPath](const auto& category)
				{
					// Add next part
					if (categoryPath.IsEmpty())
					{
						categoryPath = Utility::String::FromWxStringView(category);
					}
					else
					{
						categoryPath = Utility::String::ConcatWithSeparator(wxS('/'), categoryPath, Utility::String::FromWxStringView(category));
					}

					// See if that branch already exist
					auto it = m_Categories.find(categoryPath);
					if (it != m_Categories.end())
					{
						parent = &it->second;
					}
					else
					{
						// No branch, create one
						auto[it, inserted] = m_Categories.insert_or_assign(categoryPath, categoryPath);

						// Attach to view nodes
						parent->AttachChild(&it->second, 0);
						parent = &it->second;
					}
					return true;
				});
			}
			parent->AttachChild(&item, parent->GetChildrenCount());
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
