#include "pch.hpp"
#include "OptionSerializer.h"
#include "../AppOption.h"
#include "../IWorkspaceContainer.h"
#include "OptionDatabase.h"
#include <kxf/Serialization/XML.h>
#include <kxf/UI/Controls/DataView.h>
#include <kxf/UI/Controls/DataView/MainWindow.h>
#include <kxf/UI/Windows/SplitterWindow.h>

namespace
{
	int FromDIPX(const wxWindow& window, int value)
	{
		return window.FromDIP(wxSize(value, wxDefaultCoord)).GetWidth();
	}
	int FromDIPY(const wxWindow& window, int value)
	{
		return window.FromDIP(wxSize(wxDefaultCoord, value)).GetHeight();
	}

	int ToDIPX(const wxWindow& window, int value)
	{
		return window.ToDIP(wxSize(value, wxDefaultCoord)).GetWidth();
	}
	int ToDIPY(const wxWindow& window, int value)
	{
		return window.ToDIP(wxSize(wxDefaultCoord, value)).GetHeight();
	}
}

namespace Kortex::Application::OName
{
	Kortex_DefOption(Item);
	Kortex_DefOption(UIOption);

	Kortex_DefOption(Pages);
	Kortex_DefOption(Columns);
	Kortex_DefOption(DisplayAt);
	Kortex_DefOption(Visible);
	Kortex_DefOption(Width);
	Kortex_DefOption(Current);

	Kortex_DefOption(Splitter);
	Kortex_DefOption(SashPosition);
}

namespace Kortex::Application::OptionSerializer
{
	void UILayout::DataViewLayout(AppOption& option, SerializationMode mode, kxf::UI::DataView::View& dataView)
	{
		using namespace kxf::UI::DataView;

		kxf::XMLNode columnsNode = option.GetNode().ConstructElement(OName::Columns);
		if (mode == SerializationMode::Save)
		{
			columnsNode.ClearNode();
			columnsNode.SetAttribute(OName::UIOption, true);

			for (size_t i = 0; i < dataView.GetColumnCount(); i++)
			{
				if (const Column* column = dataView.GetColumn(i))
				{
					kxf::XMLNode node = columnsNode.NewElement(OName::Item);

					node.SetAttribute(OName::DisplayAt, (int)column->GetDisplayIndex());
					node.SetAttribute(OName::Visible, column->IsVisible());
					node.SetAttribute(OName::Width, ToDIPX(dataView, column->GetWidthDescriptor().GetValue()));
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			size_t i = 0;
			for (const kxf::XMLNode& node: columnsNode.EnumChildElements())
			{
				if (Column* column = dataView.GetColumn(i))
				{
					column->SetWidth(FromDIPX(dataView, node.GetAttributeInt(OName::Width, ColumnWidth(ColumnWidth::Default).GetValue())));
					column->SetVisible(node.GetAttributeBool(OName::Visible, true));
					column->SetDisplayIndex(node.GetAttributeInt(OName::DisplayAt, i));

					i++;
				}
				break;
			};

			dataView.SendSizeEventToParent();
			dataView.SendSizeEvent();
			dataView.UpdateWindowUI();
			dataView.Update();
		}
	}
	void UILayout::SplitterLayout(AppOption& option, SerializationMode mode, kxf::UI::SplitterWindow& window)
	{
		const bool isVertical = window.GetSplitMode() == wxSPLIT_VERTICAL;

		if (mode == SerializationMode::Save)
		{
			int value = window.GetSashPosition();
			value = isVertical ? ToDIPX(window, value) : ToDIPY(window, value);

			kxf::XMLNode node = option.GetNode().ConstructElement(window.GetName());
			node.SetAttribute(OName::UIOption, true);
			node.SetAttribute(OName::SashPosition, value);
		}
		else
		{
			
			int value = option.GetNode().GetFirstChildElement(window.GetName()).GetAttributeInt(OName::SashPosition);
			int maxValue = wxSystemSettings::GetMetric(isVertical ? wxSYS_SCREEN_Y : wxSYS_SCREEN_X);

			value = isVertical ? FromDIPX(window, value) : FromDIPY(window, value);
			window.SetInitialPosition(std::clamp(value, window.GetMinimumPaneSize(), maxValue));
		}
	}
	void UILayout::WorkspaceContainerLayout(AppOption& option, SerializationMode mode, IWorkspaceContainer& container)
	{
		kxf::XMLNode pagesNode = option.GetNode().ConstructElement(OName::Pages);

		if (mode == SerializationMode::Save)
		{
			pagesNode.ClearNode();
			pagesNode.SetAttribute(OName::UIOption, true);

			container.EnumWorkspaces([&](const IWorkspace& workspace)
			{
				if (auto index = container.GetWorkspaceIndex(workspace))
				{
					kxf::XMLNode node = pagesNode.NewElement(OName::Item);
					node.SetAttribute(OName::ID, workspace.GetID());
					if (workspace.IsCurrent())
					{
						node.SetAttribute(OName::Current, true);
					}
				}
				return true;
			});
		}
		else
		{
			if (pagesNode.GetChildrenCount() == container.GetWorkspacesCount())
			{
				IWorkspace* currentWorkspace = nullptr;

				size_t index = 0;
				for (const kxf::XMLNode& node: pagesNode.EnumChildElements())
				{
					if (IWorkspace* workspace = container.GetWorkspaceByID(node.GetAttribute(OName::ID)))
					{
						container.ChangeWorkspaceIndex(*workspace, index);
						if (node.GetAttributeBool(OName::Current))
						{
							currentWorkspace = workspace;
						}
					}
					index++;
				}

				if (currentWorkspace)
				{
					container.SwitchWorkspace(*currentWorkspace);
				}
			}
		}
	}
	void UILayout::WindowGeometry(AppOption& option, SerializationMode mode, wxTopLevelWindow& window)
	{
		kxf::XMLNode geometryNode = option.GetNode().ConstructElement(wxS("WindowGeometry"));

		if (mode == SerializationMode::Save)
		{
			geometryNode.ClearNode();
			geometryNode.SetAttribute(OName::UIOption, true);

			const bool isMaximized = window.IsMaximized();
			geometryNode.NewElement(wxS("Maximized")).SetValue(isMaximized);
			if (!isMaximized)
			{
				kxf::Size size = window.GetSize();

				kxf::XMLNode sizeNode = geometryNode.NewElement(wxS("Size"));
				sizeNode.SetAttribute(wxS("Width"), ToDIPX(window, size.GetWidth()));
				sizeNode.SetAttribute(wxS("Height"), ToDIPX(window, size.GetHeight()));
			}
		}
		else
		{
			kxf::XMLNode sizeNode = geometryNode.GetFirstChildElement(wxS("Size"));

			kxf::Size size;
			size.SetWidth(FromDIPX(window, sizeNode.GetAttributeInt(wxS("Width"), wxDefaultCoord)));
			size.SetHeight(FromDIPY(window, sizeNode.GetAttributeInt(wxS("Height"), wxDefaultCoord)));
			size.DecToIfSpecified(window.GetMaxSize());
			size.IncTo(window.GetMinSize());
			window.SetSize(size);

			if (geometryNode.GetFirstChildElement(wxS("Maximized")).GetValueBool())
			{
				window.Maximize();
			}
			else
			{
				window.Center();
			}
		}
	}
}
