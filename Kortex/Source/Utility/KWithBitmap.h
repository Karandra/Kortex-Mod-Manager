#pragma once
#include "stdafx.h"

class KWithBitmap
{
	private:
		wxBitmap m_Bitmap;

	public:
		KWithBitmap(const wxBitmap& bitmap = wxNullBitmap)
			:m_Bitmap(bitmap)
		{
		}
		virtual ~KWithBitmap()
		{
		}

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
};
