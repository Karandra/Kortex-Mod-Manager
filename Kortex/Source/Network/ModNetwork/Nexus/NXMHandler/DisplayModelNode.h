#pragma once
#include "stdafx.h"
#include <DataView2/DataView2.h>

namespace Kortex
{
	class IGameInstance;
}

namespace Kortex::NetworkManager::NXMHandler
{
	class OptionStore;
}

namespace Kortex::NetworkManager::NXMHandler
{
	class DisplayModelNode: public KxDataView2::Node
	{
		private:
			wxString m_NexusID;
			OptionStore& m_Options;
			IGameInstance& m_Instance;

		private:
			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Column& column, const wxAny& value) override;

			bool Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const override;
			bool IsEnabled(const KxDataView2::Column& column) const override;

			void OpenExecutableDialog();

		public:
			DisplayModelNode(OptionStore& options, IGameInstance& instance);
	};
}
