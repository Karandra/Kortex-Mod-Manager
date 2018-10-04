#include "stdafx.h"
#include "KNotification.h"
#include "KNotificationPopup.h"
#include "KNotificationCenter.h"
#include "KAux.h"
#include "KBitmapSize.h"
#include <KxFramework/KxCoroutine.h>

void KNotification::ShowPopupWindow()
{
	KxCoroutine::Run([this](KxCoroutineBase& coroutine)
	{
		if (m_PopupWindow)
		{
			if (m_PopupWindow->IsMouseInWindow())
			{
				return coroutine.YieldWaitSeconds(1.5);
			}
			else
			{
				wxApp::GetInstance()->ScheduleForDestruction(m_PopupWindow);
				m_PopupWindow = NULL;
				return coroutine.YieldStop();
			}
		}
		else
		{
			m_PopupWindow = new KNotificationPopup(this);
			m_PopupWindow->Show();
			return coroutine.YieldWaitSeconds(3);
		}
	});
}

//////////////////////////////////////////////////////////////////////////
KNotificationSimple::KNotificationSimple(const wxString& caption, const wxString& message, KxIconType iconID)
	:m_Caption(caption), m_Message(message), m_Bitmap(iconID != KxICON_NONE ? wxArtProvider::GetMessageBoxIcon(iconID) : wxNullBitmap)
{
}
KNotificationSimple::KNotificationSimple(const wxString& caption, const wxString& message, const wxBitmap& bitmap)
	:m_Caption(caption), m_Message(message)
{
	KBitmapSize bitmapSize;
	bitmapSize.FromSystemIcon();

	if (m_Bitmap.GetWidth() != bitmapSize.GetWidth() || m_Bitmap.GetHeight() != bitmapSize.GetHeight())
	{
		m_Bitmap = bitmapSize.ScaleBitmapAspect(bitmap);
	}
	else
	{
		m_Bitmap = bitmap;
	}
}
