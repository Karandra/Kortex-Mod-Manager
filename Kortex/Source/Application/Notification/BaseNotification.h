#pragma once
#include "stdafx.h"
#include "Application/INotification.h"

namespace Kortex::Notifications
{
	class BaseNotification: public INotification
	{
		friend class DefaultNotificationCenter;
		friend class PopupWindow;

		private:
			PopupWindow* m_PopupWindow = nullptr;

		private:
			void DestroyPopupWindow() override;
			void SetPopupWindow(PopupWindow* window) override;
			PopupWindow* GetPopupWindow() const
			{
				return m_PopupWindow;
			}

		public:
			~BaseNotification();

		public:
			void ShowPopupWindow() override;
			bool HasPopupWindow() const override;

			virtual wxString GetCaption() const = 0;
			virtual wxString GetMessage() const = 0;
			virtual wxBitmap GetBitmap() const = 0;
	};
}
