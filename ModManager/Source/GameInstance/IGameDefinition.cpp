#include "pch.hpp"
#include "IGameDefinition.h"
#include "Application/IApplication.h"
#include "Application/SystemApplication.h"
#include <kxf/System/ShellOperations.h>

namespace
{
	constexpr size_t g_MaxInstanceNameLength = 64;

	std::unique_ptr<kxf::IImage2D> LoadIconFromFile(const kxf::IFileSystem& fs, const kxf::FSPath& path)
	{
		std::unique_ptr<kxf::IImage2D> image;
		if (auto stream = fs.OpenToRead(path))
		{
			image = kxf::Drawing::LoadImage(*stream);
		}
		
		if (image)
		{
			return image;
		}
		else
		{
			// If everything failed, use default application icon
			kxf::FileItem item;
			item.SetFileExtension(wxS("exe"));
			item.SetAttributes(kxf::FileAttribute::Normal);

			return kxf::Shell::GetFileIcon(item, kxf::SHGetFileIconFlag::Large).CloneImage2D();
		}
	}
}

namespace Kortex
{
	bool IGameDefinition::ValidateID(const kxf::String& id, kxf::String* validID)
	{
		if (id.IsEmpty())
		{
			return false;
		}

		// Restrict max ID length to 64 symbols
		const kxf::String forbiddenCharacters = IApplication::GetInstance().GetFileSystem(FileSystemOrigin::GameInstances).GetForbiddenPathNameCharacters();
		if (id.length() > g_MaxInstanceNameLength || id.ContainsAnyOfCharacters(forbiddenCharacters))
		{
			if (validID)
			{
				*validID = id;
				validID->Truncate(g_MaxInstanceNameLength);

				for (const auto& c: forbiddenCharacters)
				{
					validID->Replace(c, wxS('_'));
				}
			}
			return false;
		}
		return true;
	}
	std::unique_ptr<kxf::IImage2D> IGameDefinition::GetGenericIcon()
	{
		return LoadIconFromFile(IApplication::GetInstance().GetFileSystem(FileSystemOrigin::AppResources), wxS("UI/kortex-logo-icon.ico"));
	}

	std::unique_ptr<kxf::IImage2D> IGameDefinition::LoadIcon(const kxf::IFileSystem& fs, const kxf::String& path) const
	{
		return LoadIconFromFile(fs, path);
	}
	std::unique_ptr<kxf::IImage2D> IGameDefinition::LoadDefaultIcon() const
	{
		const kxf::IFileSystem& fs = IApplication::GetInstance().GetFileSystem(FileSystemOrigin::GameDefinitions);
		return LoadIconFromFile(fs, kxf::String::Format(wxS("Icons/%1.ico"), "", GetGameID().ToString()));
	}
}
