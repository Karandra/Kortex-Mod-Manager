#include "stdafx.h"
#include "NXMHandlerModel.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxMenu.h>

namespace Kortex::NetworkManager
{
	NXMHandlerModel::NXMHandlerModel()
	{
	}

	void NXMHandlerModel::OnActivate(KxDataView2::Event& event)
	{
		if (event.GetNode() && event.GetColumn())
		{
			GetView()->EditItem(*event.GetNode(), *event.GetColumn());
		}
	}

	void NXMHandlerModel::CreateView(wxWindow* parent)
	{
		using namespace KxDataView2;

		// View
		View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus|CtrlStyle::FitLastColumn);
		view->AssignModel(this);
		view->SetUniformRowHeight(view->GetDefaultRowHeight(UniformHeight::Explorer));

		// Columns
		view->AppendColumn<TextRenderer>(KTr("NetworkManager.NXMHandler.NexusID"), ColumnID::NexusID);
		{
			auto [column, r] = view->AppendColumn<TextRenderer>(KTr("NetworkManager.NXMHandler.Game"), ColumnID::Game);
			column.SetVisible(false);
		}
		{
			auto [c, r, editor] = view->AppendColumn<TextRenderer, ComboBoxEditor>(KTr("NetworkManager.NXMHandler.Target"), ColumnID::Target);
			editor.SetEditable(false);
			editor.AutoPopup();
			for (const auto& instance: IGameInstance::GetShallowInstances())
			{
				editor.AddItem(instance->GetInstanceID());
			}
		}

		// Events
		view->Bind(EvtITEM_ACTIVATED, &NXMHandlerModel::OnActivate, this);
		view->Bind(EvtCOLUMN_HEADER_RCLICK, [view](Event& event)
		{
			KxMenu menu;
			if (view->CreateColumnSelectionMenu(menu))
			{
				view->OnColumnSelectionMenu(menu);
			}
		});

		// Add items
		RefreshItems();
	}
	void NXMHandlerModel::RefreshItems()
	{
		std::unordered_set<wxString> hash;
		for (const auto& instance: IGameInstance::GetTemplates())
		{
			if (hash.insert(instance->GetVariables().GetVariable("NexusDomainName").AsString()).second)
			{
				m_Nodes.emplace_back(*instance);
			}
		}

		GetView()->GetRootNode().AttachChildren(m_Nodes);
		ItemsChanged();
	}
}
