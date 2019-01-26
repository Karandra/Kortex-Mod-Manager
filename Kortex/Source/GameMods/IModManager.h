#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "GameMods/IGameMod.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include "Network/Common.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IGameProfile;

	namespace ModManager
	{
		class Config;
	}
	namespace ModManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IModManager:
		public ManagerWithTypeInfo<IManager, ModManager::Internal::TypeInfo>,
		public KxSingletonPtr<IModManager>
	{
		friend class IGameMod;

		protected:
			intptr_t GetOrderIndex(const IGameMod& mod) const;

		protected:
			IModManager();

		public:
			virtual const ModManager::Config& GetOptions() const = 0;

			virtual std::unique_ptr<IGameMod> NewMod() = 0;
			IGameMod& EmplaceMod(std::unique_ptr<IGameMod> mod)
			{
				return *GetMods().emplace_back(std::move(mod));
			}
			IGameMod& EmplaceMod()
			{
				return *GetMods().emplace_back(NewMod());
			}

			virtual void Load() = 0;
			virtual void Save() const = 0;

			virtual const IGameMod::Vector& GetMods() const = 0;
			virtual IGameMod::Vector& GetMods() = 0;
			virtual IGameMod::RefVector GetAllMods(bool includeWriteTarget = false) = 0;
			virtual IGameMod::RefVector GetMandatoryMods() = 0;

			virtual IGameMod& GetBaseGame() = 0;
			virtual IGameMod& GetOverwrites() = 0;

			virtual void ResortMods(const IGameProfile& profile) = 0;
			virtual void ResortMods() = 0;

			virtual IGameMod* FindModByID(const wxString& modID, intptr_t* index = nullptr) const = 0;
			virtual IGameMod* FindModByName(const wxString& modName, intptr_t* index = nullptr) const = 0;
			virtual IGameMod* FindModBySignature(const wxString& signature, intptr_t* index = nullptr) const = 0;
			virtual IGameMod* FindModByNetworkID(NetworkProviderID providerID, ModID id, intptr_t* index = nullptr) const = 0;
			
			virtual bool IsModActive(const wxString& modID) const = 0;
			virtual bool ChangeModID(IGameMod& mod, const wxString& newID) = 0;
			virtual void UninstallMod(IGameMod& mod, wxWindow* window = nullptr) = 0;
			virtual void EraseMod(IGameMod& mod, wxWindow* window = nullptr) = 0;

			virtual bool MoveModsBefore(const IGameMod::RefVector& toMove, const IGameMod& anchor) = 0;
			virtual bool MoveModsAfter(const IGameMod::RefVector& toMove, const IGameMod& anchor) = 0;
			
			virtual void ExportModList(const wxString& outputFilePath) const = 0;

		public:
			virtual IVirtualFileSystem& GetVFS() = 0;

		public:
			virtual void NotifyModInstalled(IGameMod& mod) = 0;
			virtual void NotifyModUninstalled(IGameMod& mod) = 0;
			virtual void NotifyModErased(IGameMod& mod) = 0;
	};
}
