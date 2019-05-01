#pragma once
#include "stdafx.h"
#include "ResourceID.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxImageList.h>
#include <KxFramework/KxImageSet.h>

namespace Kortex
{
	class IImageProvider
	{
		public:
			virtual const KxImageList& GetImageList() const = 0;
			virtual const KxImageSet& GetImageSet() const = 0;

		public:
			wxBitmap GetBitmap(const ResourceID& resID) const;
			wxImage GetImage(const ResourceID& resID) const;
			wxIcon GetIcon(const ResourceID& resID) const;
	};
}

namespace Kortex::ImageProvider
{
	const KxImageList& GetImageList();
	const KxImageSet& GetImageSet();

	wxBitmap GetBitmap(const ResourceID& resID);
	wxImage GetImage(const ResourceID& resID);
	wxIcon GetIcon(const ResourceID& resID);
}
