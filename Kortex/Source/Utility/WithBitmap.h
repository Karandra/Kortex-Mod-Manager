#pragma once
#include "stdafx.h"

namespace Kortex::Utility
{
	class WithBitmap
	{
		private:
			wxBitmap m_Bitmap;
			bool m_NoBitmap = false;

		public:
			WithBitmap(const wxBitmap& bitmap = wxNullBitmap)
				:m_Bitmap(bitmap)
			{
			}
			virtual ~WithBitmap() = default;

		public:
			bool HasBitmap() const
			{
				return m_Bitmap.IsOk();
			}
			const wxBitmap& GetBitmap() const
			{
				return m_Bitmap;
			}
			void SetBitmap(const wxBitmap& bitmap)
			{
				m_Bitmap = bitmap;
			}
			void ResetBitmap()
			{
				m_Bitmap = wxNullBitmap;
			}

			bool CanNotHaveBitmap() const
			{
				return m_NoBitmap;
			}
			void SetCanNotHaveBitmap(bool value = true)
			{
				m_NoBitmap = value;
			}
			void ResetCanNotHaveBitmap()
			{
				m_NoBitmap = false;
			}
	};
}
