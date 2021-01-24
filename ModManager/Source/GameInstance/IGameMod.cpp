#include "pch.hpp"
#include "IGameMod.h"
#include "IGameInstance.h"
#include "Application/IApplication.h"
#include <kxf/Crypto/Crypto.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/IO/MemoryStream.h>

namespace
{
	kxf::FSPath GetRootDirectory(const kxf::String& signature)
	{
		using namespace Kortex;

		if (auto instance = IApplication::GetInstance().GetActiveGameInstance())
		{
			return instance->GetFileSystem(IGameInstance::Location::Mods).GetLookupDirectory() / signature;
		}
		return {};
	}
}

namespace Kortex
{
	kxf::String IGameMod::GetSignatureFromID(const kxf::String& id)
	{
		auto utf8 = id.ToUTF8();
		kxf::MemoryInputStream stream(utf8.data(), utf8.length());
		if (auto hash = kxf::Crypto::MD5(stream))
		{
			return hash->ToString();
		}
		return {};
	}

	kxf::IFileSystem& IGameMod::GetFileSystem(Location locationID)
	{
		/*
		switch (locationID)
		{
			case Location::Root:
			{
				return std::make_unique<kxf::NativeFileSystem>(GetRootDirectory(GetSignature()));
			}
			case Location::Content:
			{
				return std::make_unique<kxf::NativeFileSystem>(GetRootDirectory(GetSignature()) / wxS("Content"));
			}
			case Location::MetaFile:
			{
				return GetRootDirectory(GetSignature()) / wxS("Content-Meta.xml");
			}
			case Location::DescriptionFile:
			{
				return GetRootDirectory(GetSignature()) / wxS("Content-Description.txt");
			}
			case Location::PictureFile:
			{
				return GetRootDirectory(GetSignature()) / wxS("Content-Picture.dat");
			}
		};
		*/
		return kxf::FileSystem::GetNullFileSystem();
	}
}
