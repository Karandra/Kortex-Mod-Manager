#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KOperationWithProgressDialogBase;

class KLootAPI: public KxSingletonPtr<KLootAPI>
{
	private:
		enum: int
		{
			INVALID_GAME_ID = -1,
		};
		int GetLootGameID() const;
		void LoggerCallback(int level, const char* value, KOperationWithProgressDialogBase* context);

	public:
		wxString GetVersion() const;
		bool IsSupported() const;
		bool CanSortNow() const;

		wxString GetDataPath() const;
		wxString GetMasterListPath() const;
		wxString GetUserListPath() const;

		bool SortPlugins(KxStringVector& sortedList, KOperationWithProgressDialogBase* context);
};
