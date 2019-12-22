#include "stdafx.h"
#include "Drawing.h"

namespace Kortex::Utility::Drawing
{
	void ChangeLightness(wxImage& image, int alphaValue)
	{
		uint8_t* data = image.GetData();
		const size_t length = (size_t)image.GetWidth() * (size_t)image.GetHeight() * 3;
		for (size_t i = 0; i < length; i += 3)
		{
			uint8_t* r = &data[i];
			uint8_t* g = &data[i+1];
			uint8_t* b = &data[i+2];
			wxColour::ChangeLightness(r, g, b, alphaValue);
		}
	}

	wxImage ScaleImageAspect(const wxImage& source, int width, int height, wxImageResizeQuality quality)
	{
		double scale = 1;
		if (width != -1 && height != -1)
		{
			double scaleX = (double)width / source.GetWidth();
			double scaleY = (double)height / source.GetHeight();
			scale = std::min(scaleX, scaleY);
		}
		else if (width != -1)
		{
			scale = (double)width / source.GetWidth();
		}
		else if (height != -1)
		{
			scale = (double)height / source.GetHeight();
		}
		else
		{
			return wxNullImage;
		}
		return source.Scale(scale * source.GetWidth(), scale * source.GetHeight(), quality);
	}
	wxBitmap ScaleImageAspect(const wxBitmap& source, int width, int height, wxImageResizeQuality quality)
	{
		return wxBitmap(ScaleImageAspect(source.ConvertToImage(), width, height, quality), 32);
	}
}
