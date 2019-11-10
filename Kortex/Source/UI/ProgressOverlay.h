#pragma once
#include "stdafx.h"

namespace Kortex::UI
{
	class ProgressOverlay final
	{
		public:
			ProgressOverlay();
			~ProgressOverlay();

		public:
			bool IsAvailablle() const;

			void UpdateProgress(int current);
			void UpdateProgress(int64_t current, int64_t total);

		public:
			explicit operator bool() const
			{
				return IsAvailablle();
			}
			bool operator!() const
			{
				return !IsAvailablle();
			}
	};
}
