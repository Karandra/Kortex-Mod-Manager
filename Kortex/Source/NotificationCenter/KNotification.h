#pragma once
#include "stdafx.h"
class KNotificationCenter;
class KNotificationPopup;

class KNotification
{
	friend class KNotificationCenter;
	friend class KNotificationPopup;

	public:
		using Containter = std::vector<std::unique_ptr<KNotification>>;
		using RefContainter = std::vector<KNotification*>;

	private:
		KNotificationPopup* m_PopupWindow = NULL;

	private:
		bool HasPopupWindow() const
		{
			return m_PopupWindow != NULL;
		}
		void SetPopupWindow(KNotificationPopup* window)
		{
			m_PopupWindow = window;
		}
		KNotificationPopup* GetPopupWindow() const
		{
			return m_PopupWindow;
		}

	public:
		KNotification() = default;
		virtual ~KNotification() = default;

	public:
		void ShowPopupWindow();

		virtual wxString GetCaption() const = 0;
		virtual wxString GetMessage() const = 0;
		virtual wxBitmap GetBitmap() const = 0;
};

//////////////////////////////////////////////////////////////////////////
class KNotificationSimple: public KNotification
{
	private:
		wxString m_Caption;
		wxString m_Message;
		wxBitmap m_Bitmap;

	public:
		KNotificationSimple(const wxString& caption, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
		KNotificationSimple(const wxString& caption, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

	public:
		virtual wxString GetCaption() const override
		{
			return m_Caption;
		}
		virtual wxString GetMessage() const override
		{
			return m_Message;
		}
		virtual wxBitmap GetBitmap() const override
		{
			return m_Bitmap;
		}
};
