#pragma once
#include "Framework.hpp"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include <kxf/General/StaticVariablesCollection.h>
#include <kxf/General/CombinedVariablesCollection.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/Serialization/XML.h>

namespace Kortex
{
	class DefaultGameInstance: public IGameInstance
	{
		private:
			IGameDefinition* m_Definition = nullptr;

			kxf::String m_Name;
			kxf::XMLDocument m_InstanceData;
			kxf::StaticVariablesCollection m_Variables;
			kxf::CombinedVariablesCollection m_CombinedVariables;

			kxf::ScopedNativeFileSystem m_RootFS;
			kxf::ScopedNativeFileSystem m_GameFS;
			kxf::ScopedNativeFileSystem m_ModsFS;
			kxf::ScopedNativeFileSystem m_ProfilesFS;
			kxf::ScopedNativeFileSystem m_DownloadsFS;
			kxf::ScopedNativeFileSystem m_MountedGameFS;

			std::vector<std::unique_ptr<IGameProfile>> m_Profiles;
			IGameProfile* m_ActiveProfile = nullptr;

		private:
			void MakeNull();

			bool LoadInstance();
			bool ResolveDefinition(const kxf::String& name);
			void SetupVariables(const kxf::XMLNode& variablesRoot);

		public:
			DefaultGameInstance()
				:m_CombinedVariables(m_Variables)
			{
			}

		public:
			// IGameInstance
			bool IsNull() const override;
			IGameDefinition& GetDefinition() const override
			{
				return *m_Definition;
			}
			kxf::String GetName() const override
			{
				return m_Name;
			}

			kxf::XMLDocument& GetUserConfig() override
			{
				return m_InstanceData;
			}
			const kxf::XMLDocument& GetUserConfig() const override
			{
				return m_InstanceData;
			}
			bool LoadUserConfig(const kxf::IFileSystem& fileSystem) override;
			bool SaveUserConfig() override;

			kxf::IVariablesCollection& GetVariables() override
			{
				return m_CombinedVariables;
			}
			const kxf::IVariablesCollection& GetVariables() const override
			{
				return m_CombinedVariables;
			}
			kxf::String ExpandVariables(const kxf::String& variables) const override
			{
				return m_CombinedVariables.Expand(variables);
			}

			using IGameInstance::GetFileSystem;
			kxf::IFileSystem& GetFileSystem(Location locationID) override;

		public:
			IGameProfile* GetActiveProfile() const override
			{
				return m_ActiveProfile;
			}
			size_t EnumProfiles(std::function<bool(IGameProfile& profile)> func) override;

			std::unique_ptr<IGameProfile> CreateProfile(const kxf::String& profileName, const IGameProfile* baseProfile = nullptr, kxf::FlagSet<CopyFlag> copyFlags = {}) override;
			bool RemoveProfile(IGameProfile& profile) override;
			bool RenameProfile(IGameProfile& profile, const kxf::String& newName) override;
			bool SwitchActiveProfile(IGameProfile& profile) override;
	};
}
