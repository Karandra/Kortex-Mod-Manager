#include "pch.hpp"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "Application/IApplication.h"
#include "Application/SystemApplication.h"
#include <kxf/FileSystem/FileItem.h>
#include <kxf/System/ShellOperations.h>
#include <kxf/Drawing/Common.h>

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
	bool IGameInstance::ValidateID(const kxf::String& id, kxf::String* validID)
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

	kxf::object_ptr<kxf::IImage2D> IGameInstance::GetGenericIcon()
	{
		return LoadIconFromFile(IApplication::GetInstance().GetFileSystem(FileSystemOrigin::AppResources), GetGenericIconLocation());
	}
	kxf::FSPath IGameInstance::GetGenericIconLocation()
	{
		return wxS("UI/kortex-logo-icon.ico");
	}

	kxf::object_ptr<kxf::IImage2D> IGameInstance::LoadIcon(const kxf::IFileSystem& fs, const kxf::String& path) const
	{
		return LoadIconFromFile(fs, path);
	}
	kxf::FSPath IGameInstance::GetDefaultIconLocation() const
	{
		return kxf::String::Format(wxS("%1/Icons/%2.ico"), "", GetGameID().ToString());
	}

	bool IGameInstance::IsActive() const
	{
		return IApplication::GetInstance().GetActiveGameInstance() == this;
	}
	IGameProfile* IGameInstance::GetProfile(const kxf::String& id)
	{
		IGameProfile* result = nullptr;
		EnumProfiles([&](IGameProfile& profile)
		{
			if (profile.GetID() == id)
			{
				result = &profile;
				return false;
			}
			return true;
		});
		return result;
	}
}
