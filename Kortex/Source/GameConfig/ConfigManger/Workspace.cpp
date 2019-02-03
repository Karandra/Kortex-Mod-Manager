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
		using KxDataView2::CtrlStyle;
		using KxDataView2::ColumnStyle;
		using KxDataView2::TextRenderer;

		m_View = new KxDataView2::View(this, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus);
		m_View->AssignModel(new KxDataView2::Model(false));
		m_MainSizer->Add(m_View, 1, wxEXPAND);

		ColumnStyle columnStyle = ColumnStyle::Move|ColumnStyle::Size|ColumnStyle::Sort;
		m_View->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Path"), ColumnID::Path, {}, columnStyle);
		m_View->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Name"), ColumnID::Name, {}, columnStyle);
		m_View->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Type"), ColumnID::Type, {}, columnStyle);
		m_View->AppendColumn(m_Translator.GetString("ConfigManager.View.Value"), ColumnID::Value, {}, columnStyle);

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
			item.OnAttachToView();
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
