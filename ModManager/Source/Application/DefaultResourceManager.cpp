#include "pch.hpp"
#include "DefaultResourceManager.h"
#include "IApplication.h"

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar Vector[] = wxS("vector");
		static constexpr kxf::XChar Bitmap[] = wxS("bitmap");
	} g_SchemeNames;

	kxf::String GetScheme(const kxf::IImage2D& image)
	{
		if (auto format = image.GetFormat())
		{
			if (format == kxf::ImageFormat::SVG)
			{
				return g_SchemeNames.Vector;
			}
			return g_SchemeNames.Bitmap;
		}
		return {};
	}
	kxf::String MakeResourceID(const kxf::String& scheme, const kxf::String& path)
	{
		return kxf::String::Format(wxS("%1://%2"), scheme, path);
	}
	kxf::String MakeResourceID(const kxf::String& scheme, const kxf::String& server, const kxf::String& path)
	{
		return kxf::String::Format(wxS("%1://%2/%3"), scheme, server, path);
	}
}

namespace Kortex
{
	// DefaultResourceManager
	kxf::IImage2D* DefaultResourceManager::FindObjectExact(const kxf::ResourceID& id) const
	{
		auto it = m_Images.find(id);
		if (it != m_Images.end())
		{
			return it->second.get();
		}
		return nullptr;
	}
	void DefaultResourceManager::LoadFrom(const kxf::IFileSystem& fs, const kxf::FSPath& directory)
	{
		for (const auto& item: fs.EnumItems(directory, {}, kxf::FSActionFlag::LimitToFiles))
		{
			if (auto stream = fs.OpenToRead(item.GetFullPath()))
			{
				if (auto image = kxf::Drawing::LoadImage(*stream))
				{
					kxf::String id = MakeResourceID(GetScheme(*image), directory.GetName(), item.GetName());
					m_Images.insert_or_assign(id.BeforeLast('.').MakeLower(), std::move(image));
				}
			}
		}
	}

	DefaultResourceManager::DefaultResourceManager()
	{
		LoadFrom(IApplication::GetInstance().GetFileSystem(FileSystemOrigin::AppResources), "UI");
	}

	// IResourceManager
	kxf::IImage2D* DefaultResourceManager::GetImage(const kxf::ResourceID& id) const
	{
		if (!id.HasScheme())
		{
			return FindObjectExact(MakeResourceID(g_SchemeNames.Bitmap, id.GetPath()));	
		}
		return FindObjectExact(id);
	}
}
