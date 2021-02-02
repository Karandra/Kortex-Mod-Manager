#include "pch.hpp"
#include "InstanceSelectionDialog.h"
#include "GameDefinition/IGameInstance.h"
#include "GameDefinition/IGameDefinition.h"
#include "Application/IApplication.h"
#include "Application/Localization.h"

namespace
{
	enum class ColumnID
	{
		InstanceName,
		DefinitionName,
	};
}

namespace Kortex::GameDefinition::UI
{
	class InstanceSelectionDialog::DataItem: public DataView::Node
	{
		private:
			IGameInstance& m_Instance;
			mutable kxf::BitmapImage m_Icon;

		public:
			DataItem(DataView::Node& parent, IGameInstance& instance)
				:Node(&parent), m_Instance(instance)
			{
			}

		public:
			kxf::Any GetCellDisplayValue(const DataView::Column& column) const override
			{
				switch (column.GetID<ColumnID>())
				{
					case ColumnID::InstanceName:
					{
						if (!m_Icon)
						{
							int height = GetView().GetUniformRowHeight();
							m_Icon = m_Instance.GetDefinition().GetIcon().ToBitmapImage({height, height});
						}
						return DataView::ImageTextValue(m_Instance.GetName(), m_Icon);
					}
					case ColumnID::DefinitionName:
					{
						return m_Instance.GetDefinition().GetGameName();
					}
				};
				return {};
			}
			DataView::CellAttribute GetCellAttributes(const DataView::Column& column, const DataView::CellState& cellState) const override
			{
				DataView::CellAttribute attribute;
				switch (column.GetID<ColumnID>())
				{
					case ColumnID::InstanceName:
					{
						attribute.FontOptions().AddOption(DataView::CellFontOption::Bold, m_Instance.IsActive());
						break;
					}
					case ColumnID::DefinitionName:
					{
						attribute.Options().SetAlignment(kxf::Alignment::CenterVertical|kxf::Alignment::Right);
						break;
					}
				};
				return attribute;
			}

		public:
			IGameInstance& GetInstance() const
			{
				return m_Instance;
			}
	};

	class InstanceSelectionDialog::DataModel: public kxf::RTTI::Implementation<DataModel, DataView::Model, DataView::RootNode>
	{
		friend class DataItem;

		private:
			std::vector<DataItem> m_Items;
			kxf::Size m_IconSize;

		protected:
			size_t OnEnumChildren(std::function<bool(Node&)> func) override
			{
				if (func)
				{
					size_t count = 0;
					for (auto&& item: m_Items)
					{
						count++;
						if (!std::invoke(func, item))
						{
							break;
						}
					}
					return count;
				}
				else
				{
					return m_Items.size();
				}
			}
			void OnModelAttached() override
			{
				using CtrlStyle = DataView::CtrlStyle;

				decltype(auto) view = GetView();
				view.ModWindowStyle(CtrlStyle::FitLastColumn, true);
				view.ModWindowStyle(CtrlStyle::SingleSelection, true);
				view.SetUniformRowHeight(m_IconSize.GetHeight());

				view.AppendColumn<DataView::ImageTextRenderer>({}, ColumnID::InstanceName);
				view.AppendColumn<DataView::TextRenderer>({}, ColumnID::DefinitionName);
			}

		public:
			DataModel()
				:m_IconSize(kxf::System::GetMetric(kxf::SystemSizeMetric::Icon) * 2)
			{
				IApplication::GetInstance().EnumGameInstances([&](IGameInstance& instance)
				{
					m_Items.emplace_back(*this, instance);
					return true;
				});
			}

		public:
			DataView::RootNode& GetRootNode() override
			{
				return *this;
			}
			DataView::View& GetView() const override
			{
				return *DataView::Model::GetView();
			}
	};
}

namespace Kortex::GameDefinition::UI
{
	void InstanceSelectionDialog::OnSelectItem(DataView::ItemEvent& event)
	{
		m_SelectedInstance = nullptr;
		if (auto node = static_cast<const DataItem*>(event.GetNode()))
		{
			m_SelectedInstance = &node->GetInstance();
		}

		m_ButtonOK->Enable(m_SelectedInstance != nullptr);
	}

	InstanceSelectionDialog::InstanceSelectionDialog(wxWindow* parent)
		:StdDialog(parent, wxID_NONE, Localize("GameInstanceSelection.Caption"))
	{
		using kxf::Geometry::SizeRatio;

		m_View = new DataView::View(this, wxID_NONE, DataView::CtrlStyle::NoHeader);
		m_View->Bind(DataView::ItemEvent::EvtItemSelected.ToWxTag(), &InstanceSelectionDialog::OnSelectItem, this);
		m_View->AssignModel(std::make_unique<DataModel>());

		SetMinSize(FromDIP(SizeRatio::FromWidth(720, SizeRatio::r16_9)));
		SetMainIcon(kxf::StdIcon::None);
		PostCreate();

		m_ButtonOK = GetButton(kxf::StdID::OK).As<kxf::UI::Button>();
		m_ButtonOK->Disable();
	}
}
