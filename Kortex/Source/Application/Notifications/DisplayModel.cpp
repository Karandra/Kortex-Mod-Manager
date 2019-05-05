#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Notification.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>

namespace
{
	enum ColumnID
	{
		Bitmap,
		Name,
	};
}

namespace Kortex::Notifications
{
	void DisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->SetUniformRowHeight(m_BitmapSize.GetHeight() * 1.5);

		{
			auto item = GetView()->AppendColumn<KxDataViewBitmapRenderer>(KTr("Generic.Image"), ColumnID::Bitmap, KxDATAVIEW_CELL_INERT);
			m_BitmapColumn = item.GetColumn();
		}
		{
			auto item = GetView()->AppendColumn<KxDataViewHTMLRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT);
			m_MessageColumn = item.GetColumn();
		}
	}

	void DisplayModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
	{
		const INotification* notification = GetDataEntry(row);
		if (notification)
		{
			switch (column->GetID())
			{
				case ColumnID::Bitmap:
				{
					data = notification->GetBitmap();
					break;
				}
				case ColumnID::Name:
				{
					data = FormatToHTML(*notification);
					break;
				}
			};
		}
	}
	bool DisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		return false;
	}
	bool DisplayModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const
	{
		return false;
	}
	bool DisplayModel::GetCellHeightByRow(size_t row, int& height) const
	{
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		const INotification* notification = GetDataEntry(event.GetItem());
		if (notification)
		{
		}
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		if (const INotification* notification = GetDataEntry(event.GetItem()))
		{
			INotificationCenter::GetInstance()->HideNotificationsWindow();
			IEvent::CallAfter([notification]()
			{
				KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, notification->GetCaption(), notification->GetMessage());
				dialog.SetMainIcon(notification->GetBitmap());
				dialog.ShowModal();
			});
		}
	}

	wxString DisplayModel::FormatToHTML(const INotification& notification) const
	{
		return KxString::Format(wxS("<font color=\"%1\" size=\"%2\">%3</font>\r\n%4"),
								KxUtility::GetThemeColor_Caption(GetView()).ToString(KxColor::C2S::HTML),
								3,
								notification.GetCaption(),
								notification.GetMessage()
		);
	}

	DisplayModel::DisplayModel()
		:m_Data(INotificationCenter::GetInstance()->GetNotifications())
	{
		m_BitmapSize.FromSystemIcon();
		SetDataViewFlags(KxDV_NO_HEADER);
		SetDataVector(&m_Data);
	}

	void DisplayModel::OnShowWindow(wxWindow* parent)
	{
		auto SetWidth = [](KxDataViewColumn* column, int width)
		{
			column->SetWidth(width);
			column->SetMinWidth(width);
			return width;
		};

		int bitmap = SetWidth(m_BitmapColumn, m_BitmapSize.GetWidth() + 4);
		SetWidth(m_MessageColumn, parent->GetSize().GetWidth() - bitmap - 4);
	}
}
