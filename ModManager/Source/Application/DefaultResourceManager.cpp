#include "pch.hpp"
#include "DefaultResourceManager.h"
#include "IApplication.h"
#include "GameDefinition/IGameDefinition.h"
#include <kxf/System/ShellOperations.h>

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar Any[] = wxS("any");
		static constexpr kxf::XChar Vector[] = wxS("vector");
		static constexpr kxf::XChar Bitmap[] = wxS("bitmap");
	} g_SchemeNames;

	constexpr struct
	{
		static constexpr kxf::XChar UI[] = wxS("ui");
	} g_ServerNames;
}

namespace Kortex
{
	// DefaultResourceManager
	std::shared_ptr<kxf::IImage2D> DefaultResourceManager::FindObjectExact(const kxf::ResourceID& id) const
	{
		auto it = m_Images.find(id.ToString().MakeLower());
		if (it != m_Images.end())
		{
			return it->second;
		}
		return nullptr;
	}
	
	void DefaultResourceManager::LoadFrom(const kxf::IFileSystem& fs, const kxf::FSPath& directory, const kxf::String& category)
	{
		for (const auto& item: fs.EnumItems(directory, {}, kxf::FSActionFlag::LimitToFiles|kxf::FSActionFlag::Recursive))
		{
			kxf::FSPath fullPath = item.GetFullPath();
			if (auto stream = fs.OpenToRead(fullPath))
			{
				if (auto image = kxf::Drawing::LoadImage(*stream))
				{
					const bool useFullPath = !category.IsEmpty();
					kxf::String resCategory = useFullPath ? category : directory.GetName();
					kxf::String resPath = (useFullPath ? fullPath.GetAfter(fs.GetLookupDirectory()).GetFullPath() : item.GetName()).BeforeLast('.');

					kxf::ResourceID id = MakeResourceIDWithCategory(resCategory, resPath, GetSchemeFromImage(*image));
					m_Images.insert_or_assign(std::move(id), std::move(image));
				}
			}
		}
	}
	void DefaultResourceManager::LoadGameDefinitions()
	{
		for (const IGameDefinition& gameDefinition: IApplication::GetInstance().EnumGameDefinitions())
		{
			LoadFrom(gameDefinition.GetFileSystem(IGameDefinition::Location::Resources), {}, gameDefinition.GetName());
		}
	}
	void DefaultResourceManager::LoadDynamic()
	{
		m_Images.insert_or_assign(MakeResourceIDWithCategory(g_ServerNames.UI, wxS("generic-game-logo"), g_SchemeNames.Bitmap), []()
		{
			kxf::FileItem item;
			item.SetFileExtension(wxS("exe"));
			item.SetAttributes(kxf::FileAttribute::Normal);

			return std::make_shared<kxf::BitmapImage>(kxf::Shell::GetFileIcon(item, kxf::SHGetFileIconFlag::Large));
		}());
	}

	DefaultResourceManager::DefaultResourceManager()
	{
		// Load static resources
		LoadFrom(IApplication::GetInstance().GetFileSystem(FileSystemOrigin::AppResources), "UI");

		// Load resource sprovided by loaded game definitions
		LoadGameDefinitions();

		// Load resources that we can create on the fly such as those extracted from system libraries
		LoadDynamic();
	}

	// IResourceManager
	kxf::String DefaultResourceManager::GetSchemeFromImage(const kxf::IImage2D& image) const
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

	std::shared_ptr<kxf::IImage2D> DefaultResourceManager::GetImage(const kxf::ResourceID& id) const
	{
		if (id)
		{
			if (!id.HasScheme() || id.GetScheme() == g_SchemeNames.Any)
			{
				// Try vector first
				if (auto result = FindObjectExact(MakeResourceID(id.GetPath(), g_SchemeNames.Vector)))
				{
					return result;
				}

				// Fallback to bitmap
				return FindObjectExact(MakeResourceID(id.GetPath(), g_SchemeNames.Bitmap));
			}
			return FindObjectExact(id);
		}
		return nullptr;
	}
}
