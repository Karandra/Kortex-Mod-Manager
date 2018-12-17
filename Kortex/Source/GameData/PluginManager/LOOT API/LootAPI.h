#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxVersion.h>
class KOperationWithProgressDialogBase;

namespace Kortex::PluginManager
{
	class BethesdaPluginManager;
	class Config;
	class LootAPIConfig;

	class LootAPI: public KxSingletonPtr<LootAPI>
	{
		public:
			static wxString GetLibraryName();
			static KxVersion GetLibraryVersion();

		private:
			BethesdaPluginManager* m_PluginManager = nullptr;
			const Config& m_ManagerConfig;
			const LootAPIConfig& m_LootConfig;

		private:
			enum: int
			{
				INVALID_GAME_ID = -1,
			};
			int GetLootGameID() const;
			void LoggerCallback(int level, const char* value, KOperationWithProgressDialogBase* context);

		public:
			LootAPI();

		public:
			wxString GetDataPath() const;
			wxString GetMasterListPath() const;
			wxString GetUserListPath() const;

			bool CanSortNow() const;
			bool SortPlugins(KxStringVector& sortedList, KOperationWithProgressDialogBase* context);
	};
}
