#include "stdafx.h"
#include "SimpleNotification.h"
#include "Utility/BitmapSize.h"

namespace Kortex
{
	SimpleNotification::SimpleNotification(const wxString& caption, const wxString& message, KxIconType iconID)
		:m_Caption(caption), m_Message(message), m_Bitmap(iconID != KxICON_NONE ? wxArtProvider::GetMessageBoxIcon(iconID) : wxNullBitmap)
	{
	}
	SimpleNotification::SimpleNotification(const wxString& caption, const wxString& message, const wxBitmap& bitmap)
		: m_Caption(caption), m_Message(message)
	{
		Utility::BitmapSize bitmapSize;
		bitmapSize.FromSystemIcon();

		if (m_Bitmap.GetWidth() != bitmapSize.GetWidth() || m_Bitmap.GetHeight() != bitmapSize.GetHeight())
		{
			m_Bitmap = bitmapSize.ScaleMaintainRatio(bitmap);
		}
		else
		{
			m_Bitmap = bitmap;
		}
	}
}
