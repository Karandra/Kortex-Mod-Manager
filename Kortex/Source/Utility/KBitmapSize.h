#pragma once
#include "stdafx.h"

class KBitmapSize
{
	public:
		static const constexpr auto r4_3 = 4.0 / 3.0;
		static const constexpr auto r5_4 = 5.0 / 4.0;
		static const constexpr auto r16_9 = 16.0 / 9.0;
		static const constexpr auto r16_10 = 16.0 / 10.0;

	private:
		int m_Width = 0;
		int m_Height = 0;

	public:
		KBitmapSize(int width = 0, int height = 0)
			:m_Width(width), m_Height(height)
		{
		}

	public:
		void FromWidth(int width, double ratio)
		{
			m_Width = width;
			m_Height = (double)width / ratio;
		}
		void FromHeight(int height, double ratio)
		{
			m_Width = (double)height * ratio;
			m_Height = height;
		}

		int GetWidth() const
		{
			return m_Width;
		}
		int GetHeight() const
		{
			return m_Height;
		}
		
		wxSize GetSize() const
		{
			return wxSize(m_Width, m_Height);
		}
		operator wxSize() const
		{
			return GetSize();
		}

		wxBitmap ScaleBitmapAspect(const wxBitmap& bitmap, int marginsX = 0, int marginsY = 0) const;
		wxBitmap ScaleBitmapStretch(const wxBitmap& bitmap, int marginsX = 0, int marginsY = 0) const;
};
