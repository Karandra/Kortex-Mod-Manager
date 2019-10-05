#pragma once
#include "stdafx.h"
#include "GameData/IGameSave.h"
#include "Application/IManager.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	namespace SaveManager
	{
		class Config;
	}
	namespace SaveManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class ISaveManager:
		public ManagerWithTypeInfo<IManager, SaveManager::Internal::TypeInfo>,
		public KxSingletonPtr<ISaveManager>
	{
		protected:
			IGameSave::Vector m_Saves;

		public:
			ISaveManager();
			~ISaveManager();

		public:
			virtual const SaveManager::Config& GetConfig() const = 0;

			virtual std::unique_ptr<IGameSave> NewSave() const = 0;
			IGameSave& EmplaceSave()
			{
				return EmplaceSave(NewSave());
			}
			IGameSave& EmplaceSave(std::unique_ptr<IGameSave> save)
			{
				return *m_Saves.emplace_back(std::move(save));
			}
			bool RemoveSave(IGameSave& save);
			void ClearSaves()
			{
				m_Saves.clear();
			}
			IGameSave::RefVector GetSaves() const;
			
			virtual void UpdateActiveFilters(const KxStringVector& filters) = 0;
	};
}
