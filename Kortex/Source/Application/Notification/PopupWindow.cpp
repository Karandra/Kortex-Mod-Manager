#include "stdafx.h"
#include "PopupWindow.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxStaticBitmap.h>
#include <KxFramework/KxHTMLWindow.h>

namespace Kortex::Notification
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

	PopupWindow::PopupWindow(INotification* notification)
		:wxPopupWindow(KMainWindow::HasInstance() ? KMainWindow::GetInstance() : IApplication::GetInstance()->GetTopWindow(), wxBORDER_THEME), m_Notification(notification)
	{
		CreateUI();

		const int edgeMargin = std::max(KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING) * 3;
		const wxSize windowSize = FromDIP(wxSize(300, 125));
		const wxSize posMagrins = FromDIP(wxSize(edgeMargin, edgeMargin));

		int offsetX = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X) + 2 * KLC_VERTICAL_SPACING;
		int offsetY = KLC_HORIZONTAL_SPACING;

		// Adjust offset to not overlap the status bar
		if (KMainWindow* mainWindow = KMainWindow::GetInstance())
		{
			if (KxStatusBarEx* statusBar = mainWindow->GetStatusBar())
			{
				offsetY += statusBar->GetSize().GetHeight();
			}
		}

		// Add combined height of all currently visible notifications with some padding
		const size_t popupCount = INotificationCenter::GetInstance()->GetActivePopupsCount();
		if (popupCount >= 1)
		{
			offsetY += popupCount * (windowSize.GetHeight() + KLC_VERTICAL_SPACING * 2);
		}

		SetInitialSize(windowSize);
		Position(KMainWindow::GetInstance()->GetRect().GetRightBottom() - posMagrins - wxSize(offsetX, offsetY), windowSize);
	}

	bool PopupWindow::Destroy()
	{
		return wxPopupWindow::Destroy();
	}
}
