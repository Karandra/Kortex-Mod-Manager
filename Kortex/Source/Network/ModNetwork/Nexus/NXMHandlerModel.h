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
				Game,
				Target,
			};

		private:
			std::vector<NXMHandlerModelNode> m_Nodes;

		private:
			void OnActivate(KxDataView2::Event& event);

		public:
			NXMHandlerModel();

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();
	};
}
