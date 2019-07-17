#pragma once
#include "stdafx.h"
#include "Network/IDownloadManager.h"
#include "DownloadItem.h"
#include "DisplayModelNode.h"
#include <DataView2/DataView2.h>

namespace Kortex::DownloadManager
{
	class DisplayModel: public KxDataView2::ListModel, public KxDataView2::TypeAliases
	{
		private:
			IDownloadManager& m_DownloadManager;
			std::list<DisplayModelNode> m_Nodes;

		private:
			void OnContextMenu(Event& event);

		private:
			DownloadItem* GetItem(Node* node) const
			{
				DisplayModelNode* displayNode = static_cast<DisplayModelNode*>(node);
				return displayNode ? &displayNode->m_Item : nullptr;
			}

			void RemoveAll(bool installedOnly = false);
			void SetAllHidden(bool isHidden, bool installedOnly = false);
			void Install(DownloadItem& item);

		public:
			DisplayModel();

		public:
			void CreateView(wxWindow* parent);

			DisplayModelNode& OnDonwloadAdded(DownloadItem& item);
			bool OnDonwloadRemoved(DownloadItem& item);
			void RefreshItems();
	};
}
