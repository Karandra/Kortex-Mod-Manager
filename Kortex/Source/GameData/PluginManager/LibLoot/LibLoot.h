#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxVersion.h>

namespace Kortex::Utility
{
	class OperationWithProgressDialogBase;
}

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
			bool SortPlugins(KxStringVector& sortedList, Utility::OperationWithProgressDialogBase* context = nullptr);
	};
}
