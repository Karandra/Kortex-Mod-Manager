#include "stdafx.h"
#include "PopupWindow.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxStaticBitmap.h>
#include <KxFramework/KxHTMLWindow.h>

namespace Kortex::Notifications
{
	void PopupWindow::CreateUI()
	{
		IThemeManager::GetActive().ProcessWindow(this);

		wxBoxSizer* sizerH = new wxBoxSizer(wxHORIZONTAL);
		wxBitmap bitmap = m_Notification->GetBitmap();
		if (bitmap.IsOk())
		{
			KxStaticBitmap* staticBitmap = new KxStaticBitmap(this, KxID_NONE, bitmap);
			sizerH->Add(staticBitmap, 0, wxEXPAND|wxALL, KLC_HORIZONTAL_SPACING * 3);
		}
		SetSizer(sizerH);

		wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);
		sizerH->Add(sizerRight, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING * 2);

		KxLabel* caption = new KxLabel(this, KxID_NONE, m_Notification->GetCaption(), KxLABEL_CAPTION|KxLABEL_COLORED);
		caption->SetFont(caption->GetFont().MakeLarger());
		sizerRight->Add(caption, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING * 2);

		KxHTMLWindow* message = new KxHTMLWindow(this, KxID_NONE, m_Notification->GetMessage());
		message->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_DEFAULT);
		sizerRight->Add(message, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
	}

	PopupWindow::PopupWindow(const INotification& notification)
		:wxPopupWindow(IApplication::GetInstance()->GetTopWindow(), wxBORDER_THEME),
		m_Notification(&notification)
	{
		if (GetParent())
		{
			CreateUI();

			const int edgeMargin = std::max(KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING) * 3;
			m_Size = FromDIP(wxSize(300, 125));
			m_Margin = FromDIP(wxSize(edgeMargin, edgeMargin));
			SetInitialSize(m_Size);
		}
		else
		{
			m_Notification = nullptr;
		}
	}

	void PopupWindow::Popup()
	{
		if (m_Notification)
		{
			wxSize offset;
			offset.x = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X) + 2 * KLC_VERTICAL_SPACING;
			offset.y = KLC_HORIZONTAL_SPACING;

			// Adjust offset to not overlap the status bar
			if (KMainWindow* mainWindow = KMainWindow::GetInstance())
			{
				if (KxStatusBarEx* statusBar = mainWindow->GetStatusBar())
				{
					offset.y += statusBar->GetSize().GetHeight();
				}
			}

			// Add combined height of all currently visible notifications with some padding
			const size_t popupCount = INotificationCenter::GetInstance()->GetActivePopupsCount();
			if (popupCount > 1)
			{
				offset.y += popupCount * (m_Size.GetHeight() + KLC_VERTICAL_SPACING * 2);
			}

			wxPopupWindow::Position(GetParent()->GetRect().GetRightBottom() - m_Margin - offset, m_Size);
			wxPopupWindow::Show();
		}
	}
	void PopupWindow::Dismiss()
	{
		Destroy();
		m_Notification = nullptr;
	}
}
