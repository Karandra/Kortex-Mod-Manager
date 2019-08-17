#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
class KxCoroutine;

namespace Kortex::Notifications
{
	class BaseNotification: public INotification
	{
		friend class DefaultNotificationCenter;

		public:
			enum class PopupType
			{
				PopupWindow,
				Tray
			};

		private:
			std::unique_ptr<INotificationPopup> m_Popup;
			KxCoroutine* m_Coroutine = nullptr;
			PopupType m_Type = PopupType::Tray;

		private:
			void UsePopupWindow();
			void UseTray();

			void DoDestroy();

		public:
			BaseNotification() = default;
			~BaseNotification()
			{
				DoDestroy();
			}

		public:
			void Popup() override;
			bool HasPopup() const override;
			void DestroyPopup() override;
	};
}
