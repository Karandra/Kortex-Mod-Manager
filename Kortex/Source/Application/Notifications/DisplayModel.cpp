#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Notification.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>
using namespace KxDataView2;

namespace
{
	enum class ColumnRef
	{
		Icon,
		Value,
		ActionRemove,
	};
}

namespace Kortex::Notifications
{
	wxAny DisplayModel::GetValue(const Node& node, const Column& column) const
	{
		const INotification& notification = GetItem(node);
		switch (column.GetID<ColumnRef>())
		{
			case ColumnRef::ActionRemove:
			{
				return ImageProvider::GetBitmap(ImageResourceID::BellMinus);
			}
			case ColumnRef::Icon:
			{
				return notification.GetBitmap();
			}
			case ColumnRef::Value:
			{
				return FormatText(notification);
			}
		};
		return {};
	}
	ToolTip DisplayModel::GetToolTip(const Node& node, const Column& column) const
	{
		const INotification& notification = GetItem(node);
		switch (column.GetID<ColumnRef>())
		{
			case ColumnRef::ActionRemove:
			{
				return KTr("NotificationCenter.RemoveNotification");
			}
			case ColumnRef::Icon:
			case ColumnRef::Value:
			{
				ToolTip toolTip(notification.GetCaption(), notification.GetMessage());
				toolTip.SetAnchorColumn(*GetView()->GetColumnByID(ColumnRef::Icon));
				toolTip.DisplayOnlyIfClipped(*GetView()->GetColumnByID(ColumnRef::Value));
				return toolTip;
			}
		};
		return VirtualListModel::GetToolTip(node, column);
	}
	bool DisplayModel::GetAttributes(const Node& node, const Column& column, const CellState& cellState, CellAttributes& attributes) const
	{
		const INotification& notification = GetItem(node);
		switch (column.GetID<ColumnRef>())
		{
			case ColumnRef::ActionRemove:
			{
				if (cellState.IsHotTracked() && column.IsHotTracked())
				{
					attributes.Options().Enable(CellOption::HighlightItem);
					return true;
				}
				break;
			}
		};
		return false;
	}

	void DisplayModel::OnSelectItem(Event& event)
	{
		if (event.GetNode() && event.GetColumn())
		{
			INotification& notification = GetItem(*event.GetNode());
			switch (event.GetColumn()->GetID<ColumnRef>())
			{
				case ColumnRef::ActionRemove:
				{
					if (INotificationCenter::GetInstance()->RemoveNotification(notification))
					{
						RefreshItems();
						GetView()->UnselectAll();
					}
					break;
				}
			};
		}
	}
	void DisplayModel::OnActivateItem(Event& event)
	{
		if (event.GetNode() && event.GetColumn())
		{
			const INotification& notification = GetItem(*event.GetNode());
			switch (event.GetColumn()->GetID<ColumnRef>())
			{
				case ColumnRef::Icon:
				case ColumnRef::Value:
				{
					INotificationCenter::GetInstance()->HideNotificationsWindow();
					BroadcastProcessor::Get().CallAfter([&notification]()
					{
						KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, notification.GetCaption(), notification.GetMessage());
						dialog.SetMainIcon(notification.GetBitmap());
						dialog.ShowModal();
					});
					break;
				}
			};
		}
	}

	wxString DisplayModel::FormatText(const INotification& notification) const
	{
		return KxString::Format(wxS("<font color='%1'>%2</font>\r\n%3"),
								KxUtility::GetThemeColor_Caption(GetView()).ToString(KxColor::C2S::HTML),
								notification.GetCaption(),
								notification.GetMessage()
		);
	}

	DisplayModel::DisplayModel()
		:m_Notifications(INotificationCenter::GetInstance()->GetNotifications())
	{
		m_BitmapSize.FromSystemIcon();
	}

	void DisplayModel::CreateView(wxWindow* parent)
	{
		View* view = new View(parent, KxID_NONE, CtrlStyle::NoHeader);
		view->ToggleWindowStyle(wxBORDER_NONE);
		view->SetEmptyControlLabel(KTr("NotificationCenter.NoNotifications"));
		view->AssignModel(this);

		view->Bind(EvtITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		view->Bind(EvtITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		view->SetUniformRowHeight(m_BitmapSize.GetHeight() * 1.5);

		view->AppendColumn<BitmapRenderer>(wxEmptyString, ColumnRef::Icon, m_BitmapSize.GetWidth() + view->FromDIP(8));
		view->AppendColumn<HTMLRenderer>(wxEmptyString, ColumnRef::Value);
		view->AppendColumn<BitmapRenderer>(wxEmptyString, ColumnRef::ActionRemove);
	}
	void DisplayModel::OnShowWindow()
	{
		GetView()->SetFocus();
	}
	void DisplayModel::RefreshItems()
	{
		SetItemCount(m_Notifications.size());
		GetView()->UnselectAll();
		GetView()->GetColumnByID(ColumnRef::Value)->FitInside();
	}
}
