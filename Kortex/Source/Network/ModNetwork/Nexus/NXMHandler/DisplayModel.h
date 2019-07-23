#pragma once
#include "stdafx.h"
#include "DisplayModelNode.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::NetworkManager::NXMHandler
{
	class OptionStore;
}

namespace Kortex::NetworkManager::NXMHandler
{
	class DisplayModel: public KxDataView2::Model
	{
		public:
			enum class ColumnID
			{
				NexusID,
				Game,
				Target,
			};
			enum class TargetOption
			{
				None = 0,
				ExternalProgram = 1,

				MAX
			};

		private:
			std::vector<DisplayModelNode> m_Nodes;
			OptionStore& m_Options;

		private:
			void OnActivate(KxDataView2::Event& event);

		public:
			DisplayModel(OptionStore& options);

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();
	};
}
