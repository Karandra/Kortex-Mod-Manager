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
		KBitmapSize(const wxSize& size)
			:m_Width(size.GetWidth()), m_Height(size.GetHeight())
		{
		}

	public:
		KBitmapSize& FromWidth(int width, double ratio)
		{
			m_Width = width;
			m_Height = (double)width / ratio;

			return *this;
		}
		KBitmapSize& FromHeight(int height, double ratio)
		{
			m_Width = (double)height * ratio;
			m_Height = height;

			return *this;
		}
		KBitmapSize& FromSystemIcon();
		KBitmapSize& FromSystemSmallIcon();

		bool IsFullySpecified() const
		{
			return m_Width > 0 && m_Height > 0;
		}
		double GetRatio() const
		{
			if (IsFullySpecified())
			{
				return (double)m_Width / m_Height;
			}
			return 0;
		}
		double GetReversedRatio() const
		{
			if (IsFullySpecified())
			{
				return (double)m_Height / m_Width;
			}
			return 0;
		}
		int GetWidth() const
		{
			return m_Width;
		}
		int GetHeight() const
		{
			return m_Height;
		}
		
		KBitmapSize& ScaleSize(double scale)
		{
			return ScaleSize(scale, scale);
		}
		KBitmapSize& ScaleSize(double xScale, double yScale)
		{
			m_Width *= xScale;
			m_Height *= yScale;
			return *this;
		}

		wxSize GetSize() const
		{
			return wxSize(m_Width, m_Height);
		}
		operator wxSize() const
		{
			return GetSize();
		}

		wxImage ScaleMaintainRatio(const wxImage& image, int marginsX = 0, int marginsY = 0) const;
		wxBitmap ScaleMaintainRatio(const wxBitmap& bitmap, int marginsX = 0, int marginsY = 0) const;

		wxImage ScaleStretch(const wxImage& image, int marginsX = 0, int marginsY = 0) const;
		wxBitmap ScaleStretch(const wxBitmap& bitmap, int marginsX = 0, int marginsY = 0) const;
};
