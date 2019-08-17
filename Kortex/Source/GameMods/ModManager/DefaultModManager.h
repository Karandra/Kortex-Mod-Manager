#pragma once
#include "stdafx.h"
#include "GameMods/IModManager.h"
#include "GameMods/IGameMod.h"
#include "GameMods/ModEvent.h"
#include "FixedGameMod.h"
#include "MainFileSystem.h"
#include "MandatoryModEntry.h"

namespace Kortex
{
	class ProfileEvent;
}

namespace Kortex::ModManager
{
	class MirroredLocation
	{
		friend class Config;

		public:
			using Vector = std::vector<MirroredLocation>;

		private:
			KxStringVector m_Sources;
			wxString m_Target;

		private:
			bool IsOK() const
			{
				return !m_Sources.empty() && !m_Target.IsEmpty();
			}

		public:
			MirroredLocation(const KxXMLNode& parentNode);

		public:
			bool ShouldUseMultiMirror() const
			{
				return m_Sources.size() > 1;
			}

			KxStringVector GetSources() const;
			wxString GetSource() const;
			wxString GetTarget() const;
	};
	class MandatoryLocation
	{
		public:
			using Vector = std::vector<MandatoryLocation>;

		private:
			wxString m_Source;
			wxString m_Name;

		public:
			MandatoryLocation(const KxXMLNode& parentNode);

		public:
			bool IsOK() const
			{
				return !m_Source.IsEmpty();
			}

			wxString GetSource() const;
			wxString GetName() const;
	};

	class Config
	{
		private:
			MirroredLocation::Vector m_MirroredLocations;
			MandatoryLocation::Vector m_MandatoryLocations;

		public:
			void OnLoadInstance(IGameInstance& profile, const KxXMLNode& node);

		public:
			const MirroredLocation::Vector& GetMirroredLocations() const
			{
				return m_MirroredLocations;
			}
			const MandatoryLocation::Vector& GetMandatoryLocations() const
			{
				return m_MandatoryLocations;
			}
	};
}

namespace Kortex::ModManager
{
	class DefaultModManager: public IModManager
	{
		friend class MainFileSystem;

		private:
			BroadcastReciever m_BroadcastReciever;
			Config m_Config;
			MainFileSystem m_VFS;

			FixedGameMod m_BaseGame;
			FixedGameMod m_WriteTarget;
			std::vector<KMandatoryModEntry> m_MandatoryMods;

			bool m_LoadMods = false;

		private:
			IGameMod* DoCreateMod(const wxString& signature);
			void ProcessInstallMod(IGameMod& mod);
			
			void DoUninstallMod(IGameMod& mod, const bool erase);
			void ProcessEraseMod(IGameMod& mod);
			void ProcessUninstallMod(IGameMod& mod);

			void OnMountPointError(const KxStringVector& locations);
			void OnUpdateModLayoutNeeded(ModEvent& event);
			void OnModLayoutSaveNeeded(ModEvent& event);
			void OnProfileSelected(ProfileEvent& event);
			
		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;

		public:
			DefaultModManager();

		public:
			KWorkspace* GetWorkspace() const override;
			void Load() override;
			void Save() const override;

			const ModManager::Config& GetOptions() const
			{
				return m_Config;
			}

			std::unique_ptr<IGameMod> NewMod() override;
			IGameMod::RefVector GetMods(GetModsFlags flags = GetModsFlags::None) override;
			size_t GetModsCount(ModManager::GetModsFlags flags = ModManager::GetModsFlags::None) override;
			IGameMod::RefVector GetMandatoryMods() override;

			IGameMod& GetBaseGame() override
			{
				return m_BaseGame;
			}
			IGameMod& GetWriteTarget() override
			{
				return m_WriteTarget;
			}

			IGameMod* FindModByID(const wxString& modID) const override;
			IGameMod* FindModByName(const wxString& modName) const override;
			IGameMod* FindModBySignature(const wxString& signature) const override;
			IGameMod* FindModByModNetwork(const wxString& modNetworkName, NetworkModInfo modInfo) const override;
			IGameMod* FindModByModNetwork(const IModNetwork& modNetwork, NetworkModInfo modInfo) const override;
			
			bool IsModActive(const wxString& modID) const override;
			bool ChangeModID(IGameMod& mod, const wxString& newID) override;
			void ExportModList(const wxString& outputFilePath) const override;

			void InstallEmptyMod(const wxString& name) override;
			void InstallModFromFolder(const wxString& sourcePath, const wxString& name, bool linkLocation = false) override;
			void InstallModFromPackage(const wxString& packagePath) override;
			void UninstallMod(IGameMod& mod) override
			{
				DoUninstallMod(mod, false);
			}
			void EraseMod(IGameMod& mod) override
			{
				DoUninstallMod(mod, true);
			}
			
		public:
			IVirtualFileSystem& GetFileSystem() override
			{
				return m_VFS;
			}
	};
}
