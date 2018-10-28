#include "stdafx.h"
#include "KNotificationPopupList.h"
#include "KNotificationCenter.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>

enum ColumnID
{
	Bitmap,
	Name,
};

void KNotificationPopupList::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KNotificationPopupList::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KNotificationPopupList::OnActivateItem, this);
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

void KNotificationPopupList::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const KNotification* notification = GetDataEntry(row);
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
bool KNotificationPopupList::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	return false;
}
bool KNotificationPopupList::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const
{
	return false;
}
bool KNotificationPopupList::GetCellHeightByRow(size_t row, int& height) const
{
	return false;
}

void KNotificationPopupList::OnSelectItem(KxDataViewEvent& event)
{
	const KNotification* notification = GetDataEntry(event.GetItem());
	if (notification)
	{
	}
}
void KNotificationPopupList::OnActivateItem(KxDataViewEvent& event)
{
	const KNotification* notification = GetDataEntry(event.GetItem());
	if (notification)
	{
		KEvent::CallAfter([notification]()
		{
			KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, notification->GetCaption(), notification->GetMessage());

			wxIcon icon;
			icon.CopyFromBitmap(notification->GetBitmap());
			dialog.SetMainIcon(icon);

			dialog.ShowModal();
		});
	}
}

wxString KNotificationPopupList::FormatToHTML(const KNotification& notification) const
{
	return KxString::Format(wxS("<font color=\"%1\" size=\"%2\">%3</font>\r\n%4"),
							KxUtility::GetThemeColor_Caption(GetView()).GetAsString(KxC2S_HTML_SYNTAX),
							3,
							notification.GetCaption(),
							notification.GetMessage()
	);
}

KNotificationPopupList::KNotificationPopupList()
	:m_Data(KNotificationCenter::GetInstance()->GetNotifications())
{
	m_BitmapSize.FromSystemIcon();
	SetDataViewFlags(KxDV_NO_HEADER);
	SetDataVector(&m_Data);
}

void KNotificationPopupList::OnShowWindow(wxWindow* parent)
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
