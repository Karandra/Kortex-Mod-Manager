#include "stdafx.h"
#include "Utility/KBitmapSize.h"
#include "Utility/KAux.h"

namespace
{
	int ProcessMargin(int sizeValue, int margin)
	{
		return margin >= 0 ? sizeValue - margin : -1;
	}
}

KBitmapSize& KBitmapSize::FromSystemIcon()
{
	m_Width = wxSystemSettings::GetMetric(wxSYS_ICON_X);
	m_Height = wxSystemSettings::GetMetric(wxSYS_ICON_Y);

	return *this;
}
KBitmapSize& KBitmapSize::FromSystemSmallIcon()
{
	m_Width = wxSystemSettings::GetMetric(wxSYS_SMALLICON_X);
	m_Height = wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y);

	return *this;
}

wxImage KBitmapSize::ScaleMaintainRatio(const wxImage& image, int marginsX, int marginsY) const
{
	return KAux::ScaleImageAspect(image, ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY));
}
wxBitmap KBitmapSize::ScaleMaintainRatio(const wxBitmap& bitmap, int marginsX, int marginsY) const
{
	return wxBitmap(ScaleMaintainRatio(bitmap.ConvertToImage(), marginsX, marginsY), 32);
}

wxImage KBitmapSize::ScaleStretch(const wxImage& image, int marginsX, int marginsY) const
{
	return image.Scale(ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY), wxIMAGE_QUALITY_HIGH);
}
wxBitmap KBitmapSize::ScaleStretch(const wxBitmap& bitmap, int marginsX, int marginsY) const
{
	return wxBitmap(ScaleStretch(bitmap.ConvertToImage()), 32);
}
