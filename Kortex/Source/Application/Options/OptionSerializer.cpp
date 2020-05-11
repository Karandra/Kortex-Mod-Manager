#include "stdafx.h"
#include "OptionSerializer.h"
#include <Kortex/Application.hpp>
#include "Application/Options/OptionDatabase.h"
#include <KxFramework/KxDataView.h>
#include <KxFramework/DataView2/DataView2.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>
#include <KxFramework/KxSplitterWindow.h>

namespace
{
	int FromDIPX(const wxWindow* window, int value)
	{
		return window->FromDIP(wxSize(value, wxDefaultCoord)).GetWidth();
	}
	int FromDIPY(const wxWindow* window, int value)
	{
		return window->FromDIP(wxSize(wxDefaultCoord, value)).GetHeight();
	}

	int ToDIPX(const wxWindow* window, int value)
	{
		return window->ToDIP(wxSize(value, wxDefaultCoord)).GetWidth();
	}
	int ToDIPY(const wxWindow* window, int value)
	{
		return window->ToDIP(wxSize(wxDefaultCoord, value)).GetHeight();
	}
}

namespace Kortex::Application::OName
{
	KortexDefOption(Item);
	KortexDefOption(UIOption);

	KortexDefOption(Pages);
	KortexDefOption(Columns);
	KortexDefOption(DisplayAt);
	KortexDefOption(Visible);
	KortexDefOption(Width);
	KortexDefOption(Current);

	KortexDefOption(Splitter);
	KortexDefOption(SashPosition);
}

namespace Kortex::Application::OptionSerializer
{
	void UILayout::DataViewLayout(AppOption& option, SerializationMode mode, KxDataViewCtrl* dataView)
	{
		const int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		KxXMLNode columnsNode = option.GetNode().ConstructElement(OName::Columns);

		if (mode == SerializationMode::Save)
		{
			columnsNode.ClearNode();
			columnsNode.SetAttribute(OName::UIOption, true);

			for (size_t i = 0; i < dataView->GetColumnCount(); i++)
			{
				KxXMLNode node = columnsNode.NewElement(OName::Item);
				const KxDataViewColumn* column = dataView->GetColumn(i);

				node.SetAttribute(OName::DisplayAt, (int64_t)dataView->GetColumnPosition(column));
				node.SetAttribute(OName::Visible, column->IsVisible());
				node.SetAttribute(OName::Width, ToDIPX(dataView, column->GetWidth()));
			}
		}
		else
		{
			wxArrayInt indexes;
			indexes.resize(dataView->GetColumnCount());

			KxXMLNode node = columnsNode.GetFirstChildElement();
			for (size_t i = 0; i < dataView->GetColumnCount(); i++)
			{
				indexes[i] = i;

				if (node.IsOK())
				{
					KxDataViewColumn* column = dataView->GetColumn(i);

					if (column->IsResizeable())
					{
						int width = FromDIPX(dataView, node.GetAttributeInt(OName::Width, wxDefaultCoord));
						if (width > 0)
						{
							column->SetWidth(std::clamp(width, column->GetMinWidth(), screenWidth));
						}
					}
					column->SetVisible(node.GetAttributeBool(OName::Visible, true));
					indexes[i] = node.GetAttributeInt(OName::DisplayAt, i);

					node = node.GetNextSiblingElement();
				}
			}

			if (wxHeaderCtrl* header = dataView->GetHeaderCtrl())
			{
				header->SetColumnsOrder(indexes);
			}

			dataView->UpdateWindowUI();
			dataView->SendSizeEventToParent();
			dataView->SendSizeEvent();
			dataView->Update();
		}
	}
	void UILayout::DataView2Layout(AppOption& option, SerializationMode mode, KxDataView2::View* dataView)
	{
		using namespace KxDataView2;

		KxXMLNode columnsNode = option.GetNode().ConstructElement(OName::Columns);
		if (mode == SerializationMode::Save)
		{
			columnsNode.ClearNode();
			columnsNode.SetAttribute(OName::UIOption, true);

			for (size_t i = 0; i < dataView->GetColumnCount(); i++)
			{
				if (const Column* column = dataView->GetColumn(i))
				{
					KxXMLNode node = columnsNode.NewElement(OName::Item);

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
			for (KxXMLNode node = columnsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				if (Column* column = dataView->GetColumn(i))
				{
					column->SetWidth(FromDIPX(dataView, node.GetAttributeInt(OName::Width, ColumnWidth::Default)));
					column->SetVisible(node.GetAttributeBool(OName::Visible, true));
					column->SetDisplayIndex(node.GetAttributeInt(OName::DisplayAt, i));

					i++;
				}
				else
				{
					break;
				}
			}

			dataView->SendSizeEventToParent();
			dataView->SendSizeEvent();
			dataView->UpdateWindowUI();
			dataView->Update();
		}
	}
	void UILayout::SplitterLayout(AppOption& option, SerializationMode mode, KxSplitterWindow* window)
	{
		const bool isVertical = window->GetSplitMode() == wxSPLIT_VERTICAL;

		if (mode == SerializationMode::Save)
		{
			int value = window->GetSashPosition();
			value = isVertical ? ToDIPX(window, value) : ToDIPY(window, value);

			KxXMLNode node = option.GetNode().ConstructElement(window->GetName());
			node.SetAttribute(OName::UIOption, true);
			node.SetAttribute(OName::SashPosition, value);
		}
		else
		{
			
			int value = option.GetNode().GetFirstChildElement(window->GetName()).GetAttributeInt(OName::SashPosition);
			int maxValue = wxSystemSettings::GetMetric(isVertical ? wxSYS_SCREEN_Y : wxSYS_SCREEN_X);

			value = isVertical ? FromDIPX(window, value) : FromDIPY(window, value);
			window->SetInitialPosition(std::clamp(value, window->GetMinimumPaneSize(), maxValue));
		}
	}
	void UILayout::WorkspaceContainerLayout(AppOption& option, SerializationMode mode, IWorkspaceContainer& container)
	{
		KxXMLNode pagesNode = option.GetNode().ConstructElement(OName::Pages);

		if (mode == SerializationMode::Save)
		{
			pagesNode.ClearNode();
			pagesNode.SetAttribute(OName::UIOption, true);

			for (const IWorkspace* workspace: container.EnumWorkspaces())
			{
				if (auto index = container.GetWorkspaceIndex(*workspace))
				{
					KxXMLNode node = pagesNode.NewElement(OName::Item);
					node.SetAttribute(OName::ID, workspace->GetID());
					if (workspace->IsCurrent())
					{
						node.SetAttribute(OName::Current, true);
					}
				}
			}
		}
		else
		{
			if (pagesNode.GetChildrenCount() == container.GetWorkspaceCount())
			{
				IWorkspace* currentWorkspace = nullptr;

				size_t index = 0;
				for (KxXMLNode node = pagesNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
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
	void UILayout::WindowGeometry(AppOption& option, SerializationMode mode, wxTopLevelWindow* window)
	{
		KxXMLNode geometryNode = option.GetNode().ConstructElement(wxS("WindowGeometry"));

		if (mode == SerializationMode::Save)
		{
			geometryNode.ClearNode();
			geometryNode.SetAttribute(OName::UIOption, true);

			const bool isMaximized = window->IsMaximized();
			geometryNode.NewElement(wxS("Maximized")).SetValue(isMaximized);
			if (!isMaximized)
			{
				wxSize size = window->GetSize();

				KxXMLNode sizeNode = geometryNode.NewElement(wxS("Size"));
				sizeNode.SetAttribute(wxS("Width"), ToDIPX(window, size.GetWidth()));
				sizeNode.SetAttribute(wxS("Height"), ToDIPX(window, size.GetHeight()));
			}
		}
		else
		{
			KxXMLNode sizeNode = geometryNode.GetFirstChildElement(wxS("Size"));

			wxSize size;
			size.SetWidth(FromDIPX(window, sizeNode.GetAttributeInt(wxS("Width"), wxDefaultCoord)));
			size.SetHeight(FromDIPY(window, sizeNode.GetAttributeInt(wxS("Height"), wxDefaultCoord)));
			size.DecToIfSpecified(window->GetMaxSize());
			size.IncTo(window->GetMinSize());
			window->SetSize(size);

			if (geometryNode.GetFirstChildElement(wxS("Maximized")).GetValueBool())
			{
				window->Maximize();
			}
			else
			{
				window->Center();
			}
		}
	}
}
