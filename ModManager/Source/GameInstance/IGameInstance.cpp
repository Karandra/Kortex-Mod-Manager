#include "pch.hpp"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "Application/IApplication.h"
#include "Application/SystemApplication.h"
#include <kxf/FileSystem/FileItem.h>
#include <kxf/System/ShellOperations.h>
#include <kxf/Drawing/SizeRatio.h>

namespace
{
	constexpr size_t g_MaxInstanceNameLength = 64;

	wxBitmap LoadIconFromFile(const kxf::FSPath& path)
	{
		wxBitmap bitmap(path.GetFullPath(), wxBITMAP_TYPE_ANY);
		if (bitmap.IsOk())
		{
			auto ratio = kxf::Geometry::SizeRatio::FromSystemIcon();
			if (bitmap.GetWidth() != ratio.GetWidth() || bitmap.GetHeight() != ratio.GetHeight())
			{
				ratio = ratio.ScaleMaintainRatio(bitmap.GetSize());
				bitmap = bitmap.ConvertToImage().Rescale(ratio.GetWidth(), ratio.GetHeight(), wxIMAGE_QUALITY_HIGH);
			}
		}
		else
		{
			kxf::FileItem item;
			item.SetFileExtension("exe");
			item.SetAttributes(kxf::FileAttribute::Normal);

			bitmap = kxf::Shell::GetFileIcon(item, kxf::SHGetFileIconFlag::Large);
		}
		return bitmap;
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
		const kxf::String forbiddenCharacters = kxf::FileSystem::GetForbiddenChars();
		if (id.length() > g_MaxInstanceNameLength || id.Contains(forbiddenCharacters))
		{
			if (validID)
			{
				*validID = id;

				for (const auto& c: forbiddenCharacters)
				{
					validID->Replace(c, wxS(""));
				}
				validID->Truncate(g_MaxInstanceNameLength);
			}
			return false;
		}
		return true;
	}

	wxBitmap IGameInstance::GetGenericIcon()
	{
		return LoadIconFromFile(GetGenericIconLocation());
	}
	kxf::FSPath IGameInstance::GetGenericIconLocation()
	{
		return IApplication::GetInstance().GetDataDirectory() / "UI/kortex-logo-icon.ico";
	}

	wxBitmap IGameInstance::LoadIcon(const kxf::String& path) const
	{
		return LoadIconFromFile(path);
	}
	kxf::FSPath IGameInstance::GetDefaultIconLocation() const
	{
		return kxf::String::Format(wxS("%1/Icons/%2.ico"), "", GetGameID().ToString());
	}

	bool IGameInstance::IsActive() const
	{
		return IApplication::GetInstance().GetActiveGameInstance() == this;
	}
	kxf::object_ptr<IGameProfile> IGameInstance::GetProfile(const kxf::String& id)
	{
		kxf::object_ptr<IGameProfile> result;
		EnumProfiles([&](kxf::object_ptr<IGameProfile> profile)
		{
			if (profile->GetID() == id)
			{
				result = std::move(profile);
				return false;
			}
			return true;
		});
		return result;
	}
}
