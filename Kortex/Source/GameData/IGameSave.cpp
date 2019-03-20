#include "stdafx.h"
#include "IGameSave.h"

namespace Kortex
{
	wxImage IGameSave::ReadImageRGB(const KxUInt8Vector& rgbData, int width, int height, int alphaOverride, bool isStaticData)
	{
		wxImage image(width, height, static_cast<unsigned char*>(const_cast<uint8_t*>(rgbData.data())), isStaticData);
		if (alphaOverride > 0)
		{
			image.InitAlpha();
			uint8_t* alpha = image.GetAlpha();
			for (size_t i = 0; i < (size_t)width * height; i++)
			{
				alpha[i] = alphaOverride;
			}
		}
		return image;
	}
	wxImage IGameSave::ReadImageRGBA(const KxUInt8Vector& rgbaData, int width, int height, int alphaOverride)
	{
		// Create image with correct size and copy RGB and alpha values separately
		wxImage image(width, height);
		if (!image.HasAlpha())
		{
			image.InitAlpha();
		}
		unsigned char* data = image.GetData();
		unsigned char* alphaData = image.GetAlpha();

		size_t rgbaIndex = 0;
		size_t alphaIndex = 0;
		const size_t length = (size_t)width * (size_t)height * 3;
		for (size_t rgbIndex = 0; rgbIndex < length;)
		{
			data[rgbIndex] = rgbaData[rgbaIndex];
			data[rgbIndex+1] = rgbaData[rgbaIndex+1];
			data[rgbIndex+2] = rgbaData[rgbaIndex+2];
			alphaData[alphaIndex] = alphaOverride > 0 ? alphaOverride : rgbaData[rgbaIndex+3];

			rgbIndex += 3;
			rgbaIndex += 4;
			alphaIndex++;
		}
		return image;
	}
}
