#pragma once
#include "stdafx.h"
#include "GameMods/IModManager.h"
#include "GameMods/IGameMod.h"
#include "FixedGameMod.h"
#include "MainFileSystem.h"
#include "MandatoryModEntry.h"
#include <Kortex/Events.hpp>
class IGameProfile;

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
			Config m_Config;
			MainFileSystem m_VFS;

			IGameMod::Vector m_Mods;
			FixedGameMod m_BaseGame;
			FixedGameMod m_WriteTarget;
			std::vector<KMandatoryModEntry> m_MandatoryMods;

		protected:
			void DoUninstallMod(IGameMod& mod, wxWindow* window, const bool erase);
			IGameMod* DoCreateMod(const wxString& signature);
			
		private:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

			void OnMountPointError(const KxStringVector& locations);
			void OnModFilesChanged(ModEvent& event);

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
			IGameMod::Vector& GetMods() override
			{
				return m_Mods;
			}
			IGameMod::RefVector GetAllMods(bool activeOnly = false, bool includeWriteTarget = false) override;
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
			void UninstallMod(IGameMod& mod, wxWindow* window = nullptr) override
			{
				DoUninstallMod(mod, window, false);
			}
			void EraseMod(IGameMod& mod, wxWindow* window = nullptr) override
			{
				DoUninstallMod(mod, window, true);
			}

			void ExportModList(const wxString& outputFilePath) const override;
			
			IVirtualFileSystem& GetFileSystem() override
			{
				return m_VFS;
			}

		public:
			void NotifyModInstalled(IGameMod& mod) override;
			void NotifyModUninstalled(IGameMod& mod) override;
			void NotifyModErased(IGameMod& mod) override;
	};
}
