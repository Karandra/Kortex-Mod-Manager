#pragma once
#include "stdafx.h"
#include "BaseNotification.h"

namespace Kortex
{
	class SimpleNotification: public Notification::BaseNotification
	{
		private:
			wxString m_Caption;
			wxString m_Message;
			wxBitmap m_Bitmap;

		public:
			SimpleNotification(const wxString& caption, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			SimpleNotification(const wxString& caption, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

		public:
			wxString GetCaption() const override
			{
				return m_Caption;
			}
			wxString GetMessage() const override
			{
				return m_Message;
			}
			wxBitmap GetBitmap() const override
			{
				return m_Bitmap;
			}
	};
}
