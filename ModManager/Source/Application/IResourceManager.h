#pragma once
#include "Framework.hpp"
#include <kxf/General/ResourceID.h>
#include <kxf/Drawing/IBitmapImage.h>
#include <kxf/Drawing/BitmapImage.h>

namespace kxf
{
	class GDIBitmap;
}

namespace Kortex
{
	class KORTEX_API IResourceManager: public kxf::RTTI::Interface<IResourceManager>
	{
		KxRTTI_DeclareIID(IResourceManager, {0x8ef5a0d8, 0x2aaf, 0x453d, {0x96, 0x9b, 0x8d, 0x69, 0x6b, 0x14, 0x32, 0x2e}});

		public:
			static kxf::ResourceID MakeResourceID(const kxf::String& path, const kxf::String& scheme = {});
			static kxf::ResourceID MakeResourceIDWithCategory(const kxf::String& category, const kxf::String& path, const kxf::String& scheme = {});

		public:
			virtual kxf::String GetSchemeFromImage(const kxf::IImage2D& image) const = 0;

		public:
			virtual std::shared_ptr<kxf::IImage2D> GetImage(const kxf::ResourceID& id) const = 0;

			virtual kxf::BitmapImage GetBitmapImage(const kxf::ResourceID& id, const kxf::Size& size = kxf::Size::UnspecifiedSize()) const = 0;
			virtual kxf::GDIBitmap GetGDIImage(const kxf::ResourceID& id, const kxf::Size& size = kxf::Size::UnspecifiedSize()) const = 0;
	};
}
