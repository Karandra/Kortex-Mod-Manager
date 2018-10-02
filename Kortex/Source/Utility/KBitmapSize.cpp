#include "stdafx.h"
#include "KBitmapSize.h"
#include "KAux.h"

namespace
{
	int ProcessMargin(int sizeValue, int margin)
	{
		return margin >= 0 ? sizeValue - margin : -1;
	}
}

wxBitmap KBitmapSize::ScaleBitmapAspect(const wxBitmap& bitmap, int marginsX, int marginsY) const
{
	return KAux::ScaleImageAspect(bitmap, ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY));
}
wxBitmap KBitmapSize::ScaleBitmapStretch(const wxBitmap& bitmap, int marginsX, int marginsY) const
{
	return wxBitmap(bitmap.ConvertToImage().Rescale(ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY), wxIMAGE_QUALITY_HIGH), 32);
}
