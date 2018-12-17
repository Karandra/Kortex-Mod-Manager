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

		const int edgeMargin = KLC_VERTICAL_SPACING * 3;
		const wxSize size = FromDIP(wxSize(300, 125));
		const wxSize posMagrins = FromDIP(wxSize(edgeMargin, edgeMargin));

		int offsetY = KLC_VERTICAL_SPACING;
		if (KMainWindow* mainWindow = KMainWindow::GetInstance())
		{
			if (KxStatusBarEx* statusBar = mainWindow->GetStatusBar())
			{
				offsetY += statusBar->GetSize().GetHeight();
			}
		}

		size_t popupCount = INotificationCenter::GetInstance()->GetActivePopupsCount();
		if (popupCount >= 1)
		{
			offsetY += popupCount * (size.GetHeight() + KLC_VERTICAL_SPACING * 2);
		}

		SetInitialSize(size);
		Position(KMainWindow::GetInstance()->GetRect().GetRightBottom() - posMagrins - wxSize(0, offsetY), size);
	}

	bool PopupWindow::Destroy()
	{
		return wxPopupWindow::Destroy();
	}
}
