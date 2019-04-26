#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "GameMods/IGameMod.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include "Network/Common.h"
#include "Network/NetworkModInfo.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IGameProfile;
	class IModNetwork;

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
		protected:
			void RecalculatePriority(size_t startAt = 0);
			void SortByPriority();

		protected:
			IModManager();

		public:
			virtual const ModManager::Config& GetOptions() const = 0;

			virtual std::unique_ptr<IGameMod> NewMod() = 0;
			IGameMod& EmplaceMod()
			{
				return EmplaceMod(NewMod());
			}
			IGameMod& EmplaceMod(std::unique_ptr<IGameMod> mod)
			{
				auto& mods = GetMods();
				mod->SetPriority(mods.size());
				return *mods.emplace_back(std::move(mod));
			}

			virtual void Load() = 0;
			virtual void Save() const = 0;

			virtual IGameMod::Vector& GetMods() = 0;
			virtual IGameMod::RefVector GetAllMods(bool activeOnly = false, bool includeWriteTarget = false) = 0;
			virtual IGameMod::RefVector GetMandatoryMods() = 0;

			virtual IGameMod& GetBaseGame() = 0;
			virtual IGameMod& GetWriteTarget() = 0;

			void ResortMods();
			void ResortMods(const IGameProfile& profile);

			bool MoveModsBefore(const IGameMod::RefVector& movedMods, const IGameMod& anchor);
			bool MoveModsAfter(const IGameMod::RefVector& movedMods, const IGameMod& anchor);
			bool ChangeModPriority(IGameMod& movedMod, intptr_t targetPriority);

			virtual IGameMod* FindModByID(const wxString& modID) const = 0;
			virtual IGameMod* FindModByName(const wxString& modName) const = 0;
			virtual IGameMod* FindModBySignature(const wxString& signature) const = 0;
			virtual IGameMod* FindModByModNetwork(const wxString& sourceName, NetworkModInfo modInfo) const = 0;
			virtual IGameMod* FindModByModNetwork(const IModNetwork& modNetwork, NetworkModInfo modInfo) const = 0;
			
			virtual bool IsModActive(const wxString& modID) const = 0;
			virtual bool ChangeModID(IGameMod& mod, const wxString& newID) = 0;
			virtual void UninstallMod(IGameMod& mod, wxWindow* window = nullptr) = 0;
			virtual void EraseMod(IGameMod& mod, wxWindow* window = nullptr) = 0;

			virtual void ExportModList(const wxString& outputFilePath) const = 0;

		public:
			virtual IVirtualFileSystem& GetFileSystem() = 0;

		public:
			virtual void NotifyModInstalled(IGameMod& mod) = 0;
			virtual void NotifyModUninstalled(IGameMod& mod) = 0;
			virtual void NotifyModErased(IGameMod& mod) = 0;
	};
}
