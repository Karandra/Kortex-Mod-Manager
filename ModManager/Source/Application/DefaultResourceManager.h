#pragma once
#include "Framework.hpp"
#include "IResourceManager.h"

namespace Kortex
{
	class KORTEX_API DefaultResourceManager: public kxf::RTTI::DynamicImplementation<DefaultResourceManager, IResourceManager>
	{
		private:
			std::unordered_map<kxf::ResourceID, std::shared_ptr<kxf::IImage2D>> m_Images;
			kxf::InterpolationQuality m_InterpolationQuality = kxf::InterpolationQuality::Default;

		private:
			std::shared_ptr<kxf::IImage2D> FindObjectExact(const kxf::ResourceID& id) const;

			void LoadFrom(const kxf::IFileSystem& fs, const kxf::FSPath& directory, const kxf::String& category = {});
			void LoadGameDefinitions();
			void LoadDynamic();

		public:
			DefaultResourceManager();
			DefaultResourceManager(const DefaultResourceManager&) = delete;

		public:
			// IResourceManager
			kxf::String GetSchemeFromImage(const kxf::IImage2D& image) const override;

		public:
			std::shared_ptr<kxf::IImage2D> GetImage(const kxf::ResourceID& id) const;

			kxf::BitmapImage GetBitmapImage(const kxf::ResourceID& id, const kxf::Size& size = kxf::Size::UnspecifiedSize()) const
			{
				if (auto image = GetImage(id))
				{
					return image->ToBitmapImage(size, m_InterpolationQuality);
				}
				return {};
			}

		public:
			DefaultResourceManager& operator=(const DefaultResourceManager&) = delete;
	};
}
