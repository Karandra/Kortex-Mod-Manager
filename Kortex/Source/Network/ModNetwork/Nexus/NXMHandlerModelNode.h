#pragma once
#include "stdafx.h"
#include <DataView2/DataView2.h>

namespace Kortex
{
	class IGameInstance;
}

namespace Kortex::NetworkManager
{
	class NXMHandlerModelNode: public KxDataView2::Node
	{
		private:
			IGameInstance& m_Instance;

		private:
			wxAny GetValue(const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Column& column, const wxAny& value) override;

			bool IsEnabled(const KxDataView2::Column& column) const override;

		public:
			NXMHandlerModelNode(IGameInstance& instance)
				:m_Instance(instance)
			{
			}
	};
}
