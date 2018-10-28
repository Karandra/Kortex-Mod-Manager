#pragma once
#include "stdafx.h"
class KNotificationCenter;
class KNotificationPopup;

class KNotification
{
	friend class KNotificationCenter;
	friend class KNotificationPopup;

	public:
		using Vector = std::vector<std::unique_ptr<KNotification>>;
		using RefVector = std::vector<KNotification*>;

	private:
		KNotificationPopup* m_PopupWindow = NULL;

	private:
		void SetPopupWindow(KNotificationPopup* window);
		void DestroyPopupWindow();
		KNotificationPopup* GetPopupWindow() const
		{
			return m_PopupWindow;
		}

	public:
		KNotification() = default;
		virtual ~KNotification();

	public:
		void ShowPopupWindow();
		bool HasPopupWindow() const;

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
