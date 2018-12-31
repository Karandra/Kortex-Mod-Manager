#pragma once
#include "stdafx.h"
#include "GameMods/IModManager.h"
#include "GameMods/IGameMod.h"
#include "FixedGameMod.h"
#include "VirtualFileSystem.h"
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
		friend class VirtualFileSystem;

		private:
			Config m_Config;
			VirtualFileSystem m_VFS;

			IGameMod::Vector m_Mods;
			FixedGameMod m_BaseGame;
			FixedGameMod m_Overwrites;
			std::vector<KMandatoryModEntry> m_MandatoryMods;

		protected:
			void DoResortMods(const IGameProfile& profile);
			void DoUninstallMod(IGameMod& mod, wxWindow* window, const bool erase);
			IGameMod* DoCreateMod(const wxString& signature);

		private:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

			void OnMountPointsError(const KxStringVector& locations);
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
			const IGameMod::Vector& GetMods() const override
			{
				return m_Mods;
			}
			IGameMod::Vector& GetMods() override
			{
				return m_Mods;
			}
			IGameMod::RefVector GetAllMods(bool includeWriteTarget = false) override;
			IGameMod::RefVector GetMandatoryMods() override;

			IGameMod& GetBaseGame() override
			{
				return m_BaseGame;
			}
			IGameMod& GetOverwrites() override
			{
				return m_Overwrites;
			}

			void ResortMods(const IGameProfile& profile) override;
			void ResortMods() override;
		
			IGameMod* FindModByID(const wxString& modID, intptr_t* index = nullptr) const override;
			IGameMod* FindModByName(const wxString& modName, intptr_t* index = nullptr) const override;
			IGameMod* FindModBySignature(const wxString& signature, intptr_t* index = nullptr) const override;
			IGameMod* FindModByNetworkID(NetworkProviderID providerID, ModID id, intptr_t* index = nullptr) const override;
		
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

			bool MoveModsBefore(const IGameMod::RefVector& toMove, const IGameMod& anchor) override;
			bool MoveModsAfter(const IGameMod::RefVector& toMove, const IGameMod& anchor) override;
		
			void ExportModList(const wxString& outputFilePath) const override;
			
			IVirtualFileSystem& GetVFS() override
			{
				return m_VFS;
			}

		public:
			void NotifyModInstalled(IGameMod& mod) override;
			void NotifyModUninstalled(IGameMod& mod) override;
			void NotifyModErased(IGameMod& mod) override;
	};
}
