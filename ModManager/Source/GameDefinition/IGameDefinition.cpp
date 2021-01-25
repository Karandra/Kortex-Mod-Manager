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
	bool IGameDefinition::ValidateName(const kxf::String& name, kxf::String* validName)
	{
		if (name.IsEmpty())
		{
			return false;
		}

		// Restrict max ID length to 64 symbols
		const kxf::String forbiddenCharacters = IApplication::GetInstance().GetFileSystem(FileSystemOrigin::Unscoped).GetForbiddenPathNameCharacters();
		if (name.length() > g_MaxInstanceNameLength || name.ContainsAnyOfCharacters(forbiddenCharacters))
		{
			if (validName)
			{
				*validName = name;
				validName->Truncate(g_MaxInstanceNameLength);

				for (const auto& c: forbiddenCharacters)
				{
					validName->Replace(c, wxS('_'));
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

	std::unique_ptr<kxf::IImage2D> IGameDefinition::LoadIcon(const kxf::IFileSystem& fs, const kxf::FSPath& path) const
	{
		return LoadIconFromFile(fs, path);
	}
}
