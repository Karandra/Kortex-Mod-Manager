#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
class KxCoroutine;

namespace Kortex::Notifications
{
	class BaseNotification: public INotification
	{
		friend class DefaultNotificationCenter;
		friend class PopupWindow;

		private:
			PopupWindow* m_PopupWindow = nullptr;
			KxCoroutine* m_PopupWindowCoroutine = nullptr;

		private:
			void SetPopupWindow(PopupWindow* window)
			{
				m_PopupWindow = window;
			}
			PopupWindow* GetPopupWindow() const
			{
				return m_PopupWindow;
			}

		public:
			BaseNotification() = default;
			~BaseNotification();

		public:
			void ShowPopupWindow() override;
			bool HasPopupWindow() const override;
			void DestroyPopupWindow() override;
	};
}
