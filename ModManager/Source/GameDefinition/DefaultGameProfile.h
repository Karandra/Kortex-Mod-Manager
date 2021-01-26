#pragma once
#include "Framework.hpp"
#include "IGameProfile.h"
#include "IGameInstance.h"
#include <kxf/Serialization/XML.h>
#include <kxf/FileSystem/NativeFileSystem.h>

namespace Kortex
{
	class DefaultGameProfile: public IGameProfile
	{
		private:
			IGameInstance* m_OwningInstance = nullptr;

			kxf::String m_Name;
			mutable kxf::ScopedNativeFileSystem m_RootFS;
			mutable kxf::ScopedNativeFileSystem m_GameSavesFS;
			mutable kxf::ScopedNativeFileSystem m_GameConfigFS;
			mutable kxf::ScopedNativeFileSystem m_WriteTargetFS;

			kxf::XMLDocument m_ProfileData;
			std::vector<GameProfileMod> m_Mods;
			std::vector<GameProfilePlugin> m_Plugins;

		private:
			bool LoadProfile();

		public:
			DefaultGameProfile() = default;

		public:
			// IGameProfile
			bool IsNull() const override
			{
				return m_OwningInstance == nullptr || m_OwningInstance->IsNull() || m_Name.IsEmpty() || m_RootFS.IsNull();
			}
			IGameInstance& GetOwningInstance() const override
			{
				return *m_OwningInstance;
			}

			kxf::XMLDocument& GetProfileData() override
			{
				return m_ProfileData;
			}
			const kxf::XMLDocument& GetProfileData() const override
			{
				return m_ProfileData;
			}
			bool LoadProfileData(IGameInstance& owningIntsance, const kxf::IFileSystem& fileSystem) override;
			bool SaveProfileData() const override;

			kxf::String GetName() const override
			{
				return m_Name;
			}
			kxf::IFileSystem& GetLocation(Location locationID) override;

			void SyncWithCurrentState() override;
			size_t EnumMods(std::function<bool(const GameProfileMod& gameMod)> func) const override;
			size_t EnumPlugins(std::function<bool(const GameProfilePlugin& gamePlugin)> func) const override;
	};
}
