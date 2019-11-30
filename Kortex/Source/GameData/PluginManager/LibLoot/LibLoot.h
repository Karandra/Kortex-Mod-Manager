#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxVersion.h>
class KOperationWithProgressDialogBase;

namespace Kortex::PluginManager
{
	class LibLoot: public KxSingletonPtr<LibLoot>
	{
		public:
			static wxString GetLibraryName();
			static KxVersion GetLibraryVersion();

		public:
			LibLoot() = default;

		public:
			wxString GetDataPath() const;
			wxString GetMasterListPath() const;
			wxString GetUserListPath() const;

			bool CanSortNow() const;
			bool SortPlugins(KxStringVector& sortedList, KOperationWithProgressDialogBase* context = nullptr);
	};
}
