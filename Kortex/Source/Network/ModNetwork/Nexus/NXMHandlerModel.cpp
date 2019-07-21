#include "stdafx.h"
#include "NXMHandlerModel.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>

namespace Kortex::NetworkManager
{
	NXMHandlerModel::NXMHandlerModel()
	{
	}

	void NXMHandlerModel::CreateView(wxWindow* parent)
	{
		using namespace KxDataView2;

		View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus|CtrlStyle::FitLastColumn);
		view->AssignModel(this);
		view->SetUniformRowHeight(view->GetDefaultRowHeight(UniformHeight::Explorer));

		// Columns
		view->AppendColumn<TextRenderer>(KTr("NetworkManager.NXMHandler.NexusID"), ColumnID::NexusID);
		{
			auto [c, r, editor] = view->AppendColumn<TextRenderer, ComboBoxEditor>(KTr("NetworkManager.NXMHandler.Target"), ColumnID::Target);
			for (const auto& instance: IGameInstance::GetShallowInstances())
			{
				if (instance->GetVariables().HasVariable("NexusDomainName"))
				{
					editor.AddItem(instance->GetInstanceID());
				}
			}
		}

		// Add items
		RefreshItems();
	}
	void NXMHandlerModel::RefreshItems()
	{
		for (const auto& instance: IGameInstance::GetShallowInstances())
		{
			if (instance->GetVariables().HasVariable("NexusDomainName"))
			{
				m_Nodes.emplace_back(*instance);
			}
		}

		GetView()->GetRootNode().AttachChildren(m_Nodes);
		ItemsChanged();
	}
}
