#pragma once
#include "Framework.hpp"
#include "Application/IModule.h"

namespace Kortex
{
	class IGameMod;
}

namespace Kortex
{
	class IGameModManager: public kxf::RTTI::ExtendInterface<IGameModManager, IModule>
	{
		KxRTTI_DeclareIID(IGameModManager, {0xec61c172, 0xa750, 0x4be9, {0xb0, 0x9d, 0x27, 0xe8, 0x8c, 0x49, 0x36, 0xb2}});

		public:
			virtual IGameMod* CreateMod(const kxf::String& name) = 0;
			virtual kxf::String CreateSignature(const kxf::String& name) const;

			virtual kxf::Enumerator<IGameMod&> EnumMods() = 0;
			virtual kxf::Enumerator<const IGameMod&> EnumMods() const = 0;

			virtual bool MoveMod(IGameMod& movedMod, int targetOrder) = 0;
			virtual bool MoveModsBefore(kxf::Enumerator<IGameMod&> movedMods, const IGameMod& anchor) = 0;
			virtual bool MoveModsAfter(kxf::Enumerator<IGameMod&> movedMods, const IGameMod& anchor) = 0;

			virtual IGameMod* GetModByName(const kxf::String& name) const = 0;
			virtual IGameMod* GetModBySignature(const kxf::String& signature) const = 0;

			virtual void EraseMod(IGameMod& mod) = 0;
			virtual void UninstallMod(IGameMod& mod) = 0;

		public:
			virtual void MountGame() = 0;
	};
}
