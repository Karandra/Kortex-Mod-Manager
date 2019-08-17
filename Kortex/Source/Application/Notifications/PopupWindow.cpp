#include "stdafx.h"
#include "PopupWindow.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxStaticBitmap.h>
#include <KxFramework/KxHTMLWindow.h>
#include <wx/popupwin.h>

namespace Kortex::Notifications
{
	void PopupWindow::CreateUI()
	{
		m_Window = new wxPopupWindow(&m_DesktopWindow, wxBORDER_THEME),
		IThemeManager::GetActive().ProcessWindow(m_Window);

		wxBoxSizer* sizerH = new wxBoxSizer(wxHORIZONTAL);
		wxBitmap bitmap = m_Notification->GetBitmap();
		if (bitmap.IsOk())
		{
			KxStaticBitmap* staticBitmap = new KxStaticBitmap(m_Window, KxID_NONE, bitmap);
			sizerH->Add(staticBitmap, 0, wxEXPAND|wxALL, KLC_HORIZONTAL_SPACING * 3);
		}
		m_Window->SetSizer(sizerH);

		wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);
		sizerH->Add(sizerRight, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING * 2);

		KxLabel* caption = new KxLabel(m_Window, KxID_NONE, m_Notification->GetCaption(), KxLABEL_CAPTION|KxLABEL_COLORED);
		caption->SetFont(caption->GetFont().MakeLarger());
		sizerRight->Add(caption, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING * 2);

		KxHTMLWindow* message = new KxHTMLWindow(m_Window, KxID_NONE, m_Notification->GetMessage(), KxHTMLWindow::DefaultStyle|wxBORDER_NONE);
		message->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_DEFAULT);
		sizerRight->Add(message, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
	}
	void PopupWindow::DoDestroy()
	{
		if (wxWindow* window = m_Window)
		{
			m_Window = nullptr;
			window->Destroy();
		}
		if (INotification* notification = m_Notification)
		{
			m_Notification = nullptr;
		}
	}

	PopupWindow::PopupWindow(INotification& notification)
		:m_Notification(&notification), m_DesktopWindow(nullptr, KxID_NONE, ::GetShellWindow())
	{
		CreateUI();

		const int edgeMargin = std::max(KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING) * 3;
		m_Size = m_Window->FromDIP(wxSize(300, 125));
		m_Margin = m_Window->FromDIP(wxSize(edgeMargin, edgeMargin));
		m_Window->SetInitialSize(m_Size);
	}
	PopupWindow::~PopupWindow()
	{
		DoDestroy();
	}

	void PopupWindow::Popup()
	{
		if (m_Window)
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

			m_Window->Position(m_DesktopWindow.GetRect().GetRightBottom() - m_Margin - offset, m_Size);
			m_Window->Show();
		}
	}
	void PopupWindow::Dismiss()
	{
		if (m_Window)
		{
			m_Window->Hide();
		}
	}
	void PopupWindow::Destroy()
	{
		DoDestroy();
	}

	wxWindow* PopupWindow::GetWindow() const
	{
		return m_Window;
	}
}
