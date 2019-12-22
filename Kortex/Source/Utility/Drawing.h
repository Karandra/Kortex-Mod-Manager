#pragma once
#include "stdafx.h"
#include <KxFramework/KxStringUtility.h>

namespace Kortex::Utility::Drawing
{
	// Change lightness of the entire image using wxColour::ChangeLightness.
	void ChangeLightness(wxImage& image, int alphaValue);

	// Scales image to specified size maintaining aspect ratio
	wxImage ScaleImageAspect(const wxImage& source, int width = -1, int height = -1, wxImageResizeQuality quality = wxIMAGE_QUALITY_NORMAL);
	wxBitmap ScaleImageAspect(const wxBitmap& source, int width = -1, int height = -1, wxImageResizeQuality quality = wxIMAGE_QUALITY_NORMAL);
}
