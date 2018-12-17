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
		wxArrayInt indexes;
		if (mode == SerializationMode::Load)
		{
			indexes.resize(dataView->GetColumnCount());
		}

		const int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		for (size_t i = 0; i < dataView->GetColumnCount(); i++)
		{
			KxDataViewColumn* column = dataView->GetColumn(i);
			const int columnID = column->GetID();

			const wxString widthName = wxString::Format("Column[%d].Width", columnID);
			const wxString visibleName = wxString::Format("Column[%d].Visible", columnID);
			const wxString displayIndexName = wxString::Format("Column[%d].DisplayIndex", columnID);

			if (mode == SerializationMode::Save)
			{
				if (column->IsResizeable())
				{
					option.SetAttribute(widthName, column->GetWidth());
				}
				option.SetAttribute(visibleName, column->IsShown());
				option.SetAttribute(displayIndexName, (int64_t)dataView->GetColumnPosition(column));
			}
			else
			{
				if (column->IsResizeable())
				{
					int width = option.GetAttributeInt(widthName, KxDVC_DEFAULT_WIDTH);
					column->SetWidth(std::clamp(width, column->GetMinWidth(), screenWidth));
				}
				column->SetHidden(!option.GetAttributeBool(visibleName, true));
				indexes[i] = option.GetAttributeInt(displayIndexName, columnID);
			}
		}

		if (mode == SerializationMode::Load)
		{
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
		bool maximized = false;
		bool topLevel = false;
		if (window->IsTopLevel())
		{
			topLevel = true;
			maximized = window->IsMaximized();
		}

		if (mode == SerializationMode::Save)
		{
			if (topLevel)
			{
				option.SetAttribute("Maximized", window->IsMaximized());
			}
			else
			{
				wxSize tSize = window->GetSize();
				option.SetAttribute("Width", tSize.GetWidth());
				option.SetAttribute("Height", tSize.GetHeight());
			}
		}
		else
		{
			if (topLevel)
			{
				if (option.GetAttributeBool("Maximized"))
				{
					window->Maximize();
				}
				else
				{
					window->Center();
				}
			}
			else
			{
				wxSize size(option.GetAttributeInt("Width", wxDefaultCoord), option.GetAttributeInt("Height", wxDefaultCoord));
				size.DecToIfSpecified(window->GetMaxSize());
				size.IncTo(window->GetMinSize());
				window->SetSize(size);
			}
		}
	}
}
