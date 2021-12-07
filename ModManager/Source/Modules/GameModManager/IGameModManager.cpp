#include "pch.hpp"
#include "IGameModManager.h"
#include "GameDefinition/IGameMod.h"
#include <kxf/Crypto/Crypto.h>
#include <kxf/IO/MemoryStream.h>

namespace Kortex
{
	kxf::String IGameModManager::CreateSignature(const kxf::String& name) const
	{
		auto utf8 = name.ToUTF8();
		kxf::MemoryInputStream stream(utf8.data(), utf8.length());
		if (auto hash = kxf::Crypto::MD5(stream))
		{
			return hash.ToString();
		}
		return {};
	}
}
