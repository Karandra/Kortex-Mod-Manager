#pragma once
#include "stdafx.h"

namespace Kortex::Utility
{
	class BitmapSize final
	{
		public:
			static constexpr auto r4_3 = 4.0 / 3.0;
			static constexpr auto r5_4 = 5.0 / 4.0;
			static constexpr auto r16_9 = 16.0 / 9.0;
			static constexpr auto r16_10 = 16.0 / 10.0;

		private:
			int m_Width = 0;
			int m_Height = 0;

		public:
			BitmapSize(int width = 0, int height = 0)
				:m_Width(width), m_Height(height)
			{
			}
			BitmapSize(const wxSize& size)
				:m_Width(size.GetWidth()), m_Height(size.GetHeight())
			{
			}

		public:
			BitmapSize& FromWidth(int width, double ratio)
			{
				m_Width = width;
				m_Height = (double)width / ratio;

				return *this;
			}
			BitmapSize& FromHeight(int height, double ratio)
			{
				m_Width = (double)height * ratio;
				m_Height = height;

				return *this;
			}
			BitmapSize& FromSystemIcon();
			BitmapSize& FromSystemSmallIcon();

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
		
			BitmapSize& ScaleSize(double scale)
			{
				return ScaleSize(scale, scale);
			}
			BitmapSize& ScaleSize(double xScale, double yScale)
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

			wxImage ScaleMaintainRatio(const wxImage& image, int marginsX = 0, int marginsY = 0, wxImageResizeQuality quality = wxIMAGE_QUALITY_NORMAL) const;
			wxBitmap ScaleMaintainRatio(const wxBitmap& bitmap, int marginsX = 0, int marginsY = 0, wxImageResizeQuality quality = wxIMAGE_QUALITY_NORMAL) const;

			wxImage ScaleStretch(const wxImage& image, int marginsX = 0, int marginsY = 0, wxImageResizeQuality quality = wxIMAGE_QUALITY_NORMAL) const;
			wxBitmap ScaleStretch(const wxBitmap& bitmap, int marginsX = 0, int marginsY = 0, wxImageResizeQuality quality = wxIMAGE_QUALITY_NORMAL) const;
	};
}
