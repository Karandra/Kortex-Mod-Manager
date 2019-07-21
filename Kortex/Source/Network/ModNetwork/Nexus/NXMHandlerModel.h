#pragma once
#include "stdafx.h"
#include "NexusRepository.h"
#include "NXMHandlerModelNode.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::NetworkManager
{
	class NXMHandlerModel: public KxDataView2::Model
	{
		public:
			enum class ColumnID
			{
				NexusID,
				Target,
			};

		private:
			std::vector<NXMHandlerModelNode> m_Nodes;

		public:
			NXMHandlerModel();

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();
	};
}
