#include "pch.hpp"
#include "InstanceSelectionDialog.h"
#include "GameDefinition/IGameInstance.h"
#include "GameDefinition/IGameDefinition.h"
#include "Application/IApplication.h"
#include "Application/IResourceManager.h"
#include "Application/Localization.h"
#include <kxf/UI/Menus/MenuWidget.h>
#include <kxf/UI/Menus/ShellMenuWidget.h>
#include <kxf/UI/Menus/ShellMenuWidgetItem.h>
#include <kxf/UI/Events/MenuWidgetEvent.h>
#include <kxf/System/DynamicLibrary.h>

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
							auto& resourceManager = IApplication::GetInstance().GetResourceManager();

							m_Icon = resourceManager.GetBitmapImage(m_Instance.GetDefinition().GetIcon(), {height, height});
							if (!m_Icon)
							{
								m_Icon = resourceManager.GetBitmapImage(IResourceManager::MakeResourceID("ui/generic-game-logo"), {height, height});
							}
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
				auto enumerator = IApplication::GetInstance().EnumGameInstances();
				m_Items.reserve(enumerator.GetTotalCount().value_or(0));

				for (IGameInstance& instance: enumerator)
				{
					m_Items.emplace_back(*this, instance);
				};
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
	void InstanceSelectionDialog::OnContextMenu(DataView::ItemEvent& event)
	{
		using namespace kxf::Widgets;

		auto GetIcon = [](const kxf::String& name) -> kxf::BitmapImage
		{
			auto& app = IApplication::GetInstance();
			auto& resourceManager = app.GetResourceManager();

			if (auto definition = app.GetGameDefinitionByName(name))
			{
				return resourceManager.GetBitmapImage(definition->GetIcon());
			}
			return {};
		};

		if (!m_Menu)
		{
			//kxf::Drawing::SetDefaultRenderer(kxf::Drawing::GetGDIRenderer());

			m_Menu = kxf::NewWidget<MenuWidget>(nullptr);
			m_Menu->Bind(kxf::MenuWidgetEvent::EvtClick, [](kxf::MenuWidgetEvent& event)
			{
				auto item = event.GetMenuWidgetItem();
				if (item->IsCheckItem())
				{
					item->SetChecked(!item->IsChecked());
				}
				else if (item->IsRadioItem())
				{
					item->SetChecked(true);
				}
				else if (auto shellItem = item->QueryInterface<kxf::IShellMenuWidgetItem>())
				{
					wxMessageBox(shellItem->GetHelpString(), item->GetLabel());
					wxMessageBox(shellItem->GetIconString(), item->GetLabel());
					wxMessageBox(shellItem->GetCommandString(), item->GetLabel());
					//shellItem->InvokeShellCommand();
				}
				wxMessageBox(item->GetDescription(), item->GetLabel());
			});
			//m_Menu->SetFont(kxf::Font(10.0f, kxf::FontFamily::Default, kxf::FontStyle::Normal, kxf::FontWeight::Normal, "Consolas"));
			//m_Menu->SetColor(kxf::Drawing::GetStockColor(kxf::StockColor::Purple), kxf::WidgetColorFlag::Background);
			//m_Menu->SetColor(kxf::Drawing::GetStockColor(kxf::StockColor::Green), kxf::WidgetColorFlag::Text);

			m_Menu->InsertMenu(*kxf::NewWidget<MenuWidget>(m_Menu), "Вид");
			m_Menu->InsertMenu(*kxf::NewWidget<MenuWidget>(m_Menu), "Сортировка\tCtrl + Q");
			m_Menu->InsertItem("Обновить")->SetDefaultItem();
			m_Menu->InsertSeparator();
			m_Menu->InsertCheckItem("&Вставить")->SetEnabled(false);
			m_Menu->InsertCheckItem("В&ставить ярлык");
			m_Menu->InsertMenu(*kxf::NewWidget<MenuWidget>(m_Menu), "Power Shell 7")->SetIcon(GetIcon("Game.TES.Skyrim"));
			m_Menu->InsertRadioItem("Открыть в Windows Terminal");
			m_Menu->InsertRadioItem("Открыть с помощью Visual Studio")->SetIcon(GetIcon("Game.TES.SkyrimSE"));
			m_Menu->InsertRadioItem("TreeSize Free");
			m_Menu->InsertRadioItem("Следующее фоновое изображение рабочего стола");
			m_Menu->InsertSeparator();
			if (auto subMenu = kxf::NewWidget<ShellMenuWidget>(m_Menu))
			{
				m_Menu->InsertMenu(*subMenu, "Создать");

				//auto item = subMenu->InsertItem("Папку");
				//item->SetItemIcon(GetIcon("Game.TES.Skyrim"));
				//item->SetEnabled(false);

				//subMenu->InsertItem("Ярлык");
				//subMenu->InsertSeparator();
				//subMenu->InsertItem("Документ Microsoft Word");
				//subMenu->InsertItem("Презентацию Microsoft PowerPoint");
				//subMenu->InsertSeparator();
				subMenu->InitializeFromFSObject(kxf::DynamicLibrary::GetExecutingModule().GetFilePath());
			}
			m_Menu->InsertSeparator();
			m_Menu->InsertItem("Параметры экрана");
			m_Menu->InsertItem("Персонализация");

		}

		CallAfter([&]
		{
			m_Menu->Show();
		});
	}

	InstanceSelectionDialog::InstanceSelectionDialog(wxWindow* parent)
		:StdDialog(parent, wxID_NONE, Localize("GameInstanceSelection.Caption"))
	{
		using kxf::Geometry::SizeRatio;

		m_View = new DataView::View(this, wxID_NONE, DataView::CtrlStyle::NoHeader);
		m_View->Bind(DataView::ItemEvent::EvtItemSelected.ToWxTag(), &InstanceSelectionDialog::OnSelectItem, this);
		m_View->Bind(DataView::ItemEvent::EvtItemContextMenu.ToWxTag(), &InstanceSelectionDialog::OnContextMenu, this);
		m_View->AssignModel(std::make_unique<DataModel>());

		SetMinSize(FromDIP(kxf::Size(SizeRatio::FromWidth(720, SizeRatio::r16_9))));
		SetMainIcon(kxf::StdIcon::None);
		PostCreate();

		m_ButtonOK = GetButton(kxf::StdID::OK).As<kxf::UI::Button>();
		m_ButtonOK->Disable();
	}
}
