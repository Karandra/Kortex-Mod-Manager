#include "stdafx.h"
#include "OptionSerializer.h"
#include <Kortex/Application.hpp>
#include "UI/KWorkspace.h"
#include <KxFramework/KxDataView.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>
#include <KxFramework/KxSplitterWindow.h>

namespace Kortex::Application::OptionSerializer
{
	void UILayout::DataViewLayout(IAppOption& option, SerializationMode mode, KxDataViewCtrl* dataView)
	{
		const int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		KxXMLNode columnsNode = option.GetNode().QueryOrCreateElement(wxS("Columns"));

		if (mode == SerializationMode::Save)
		{
			columnsNode.ClearNode();

			for (size_t i = 0; i < dataView->GetColumnCount(); i++)
			{
				KxXMLNode node = columnsNode.NewElement(wxS("Entry"));
				const KxDataViewColumn* column = dataView->GetColumn(i);

				node.SetAttribute(wxS("DisplayAt"), (int64_t)dataView->GetColumnPosition(column));
				node.SetAttribute(wxS("Visible"), column->IsVisible());
				node.SetAttribute(wxS("Width"), column->GetWidth());
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
						int width = node.GetAttributeInt(wxS("Width"), -1);
						if (width > 0)
						{
							column->SetWidth(std::clamp(width, column->GetMinWidth(), screenWidth));
						}
					}
					column->SetVisible(node.GetAttributeBool(wxS("Visible"), true));
					indexes[i] = node.GetAttributeInt(wxS("DisplayAt"), i);

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
	void UILayout::SplitterLayout(IAppOption& option, SerializationMode mode, KxSplitterWindow* window)
	{
		if (mode == SerializationMode::Save)
		{
			option.SetAttribute(window->GetName(), window->GetSashPosition());
		}
		else
		{
			window->SetInitialPosition(option.GetAttributeInt(window->GetName(), window->GetMinimumPaneSize()));
		}
	}
	void UILayout::WindowSize(IAppOption& option, SerializationMode mode, wxTopLevelWindow* window)
	{
		KxXMLNode geometryNode = option.GetNode().QueryOrCreateElement(wxS("Geometry"));

		if (mode == SerializationMode::Save)
		{
			geometryNode.ClearNode();

			const bool isMaximized = window->IsMaximized();
			geometryNode.NewElement(wxS("Maximized")).SetValue(isMaximized);
			if (!isMaximized)
			{
				wxSize size = window->GetSize();

				KxXMLNode sizeNode = geometryNode.NewElement(wxS("Size"));
				sizeNode.SetAttribute(wxS("Width"), size.GetWidth());
				sizeNode.SetAttribute(wxS("Height"), size.GetHeight());
			}
		}
		else
		{
			KxXMLNode sizeNode = geometryNode.GetFirstChildElement(wxS("Size"));

			wxSize size;
			size.SetWidth(sizeNode.GetAttributeInt(wxS("Width"), wxDefaultCoord));
			size.SetHeight(sizeNode.GetAttributeInt(wxS("Height"), wxDefaultCoord));
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
