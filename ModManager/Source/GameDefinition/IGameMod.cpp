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
				return std::make_unique<kxf::NativeFileSystem>(GetRootDirectory(GetSignature()) / kxS("Content"));
			}
			case Location::MetaFile:
			{
				return GetRootDirectory(GetSignature()) / kxS("Content-Meta.xml");
			}
			case Location::DescriptionFile:
			{
				return GetRootDirectory(GetSignature()) / kxS("Content-Description.txt");
			}
			case Location::PictureFile:
			{
				return GetRootDirectory(GetSignature()) / kxS("Content-Picture.dat");
			}
		};
		*/
		return kxf::FileSystem::GetNullFileSystem();
	}
}
