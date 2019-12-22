#include "stdafx.h"
#include "Utility/BitmapSize.h"
#include "Utility/Drawing.h"

namespace
{
	int ProcessMargin(int sizeValue, int margin)
	{
		return margin >= 0 ? sizeValue - margin : -1;
	}
}

namespace Kortex::Utility
{
	BitmapSize& BitmapSize::FromSystemIcon()
	{
		m_Width = wxSystemSettings::GetMetric(wxSYS_ICON_X);
		m_Height = wxSystemSettings::GetMetric(wxSYS_ICON_Y);

		return *this;
	}
	BitmapSize& BitmapSize::FromSystemSmallIcon()
	{
		m_Width = wxSystemSettings::GetMetric(wxSYS_SMALLICON_X);
		m_Height = wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y);

		return *this;
	}

	wxImage BitmapSize::ScaleMaintainRatio(const wxImage& image, int marginsX, int marginsY, wxImageResizeQuality quality) const
	{
		return Drawing::ScaleImageAspect(image, ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY), quality);
	}
	wxBitmap BitmapSize::ScaleMaintainRatio(const wxBitmap& bitmap, int marginsX, int marginsY, wxImageResizeQuality quality) const
	{
		return Drawing::ScaleImageAspect(bitmap, ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY), quality);
	}

	wxImage BitmapSize::ScaleStretch(const wxImage& image, int marginsX, int marginsY, wxImageResizeQuality quality) const
	{
		return image.Scale(ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY), quality);
	}
	wxBitmap BitmapSize::ScaleStretch(const wxBitmap& bitmap, int marginsX, int marginsY, wxImageResizeQuality quality) const
	{
		return wxBitmap(ScaleStretch(bitmap.ConvertToImage(), quality), 32);
	}
}
