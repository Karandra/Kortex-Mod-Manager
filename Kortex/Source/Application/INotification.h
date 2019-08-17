#pragma once
#include "stdafx.h"

namespace Kortex
{
	class INotificationPopup
	{
		public:
			virtual ~INotificationPopup() = default;

		public:
			virtual void Popup() = 0;
			virtual void Dismiss() = 0;
			virtual void Destroy() = 0;
	};
}

namespace Kortex
{
	class INotification
	{
		public:
			using Vector = std::vector<std::unique_ptr<INotification>>;
			using RefVector = std::vector<INotification*>;

		public:
			INotification() = default;
			virtual ~INotification() = default;

		public:
			virtual void Popup() = 0;
			virtual bool HasPopup() const = 0;
			virtual void DestroyPopup() = 0;

			virtual wxString GetCaption() const = 0;
			virtual wxString GetMessage() const = 0;
			virtual wxBitmap GetBitmap() const = 0;
	};
}
