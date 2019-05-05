#pragma once
#include "stdafx.h"

namespace Kortex
{
	namespace Notifications
	{
		class PopupWindow;
	}

	class INotification
	{
		public:
			using Vector = std::vector<std::unique_ptr<INotification>>;
			using RefVector = std::vector<INotification*>;

		private:
			virtual void DestroyPopupWindow() = 0;
			virtual void SetPopupWindow(Notifications::PopupWindow* window) = 0;
			virtual Notifications::PopupWindow* GetPopupWindow() const = 0;

		public:
			INotification() = default;
			virtual ~INotification() = default;

		public:
			virtual void ShowPopupWindow() = 0;
			virtual bool HasPopupWindow() const = 0;

			virtual wxString GetCaption() const = 0;
			virtual wxString GetMessage() const = 0;
			virtual wxBitmap GetBitmap() const = 0;
	};
}
