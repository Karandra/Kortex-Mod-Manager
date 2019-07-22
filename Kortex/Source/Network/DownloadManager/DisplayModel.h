#pragma once
#include "stdafx.h"
#include "Network/IDownloadManager.h"
#include "DownloadItem.h"
#include "DisplayModelNode.h"
#include <DataView2/DataView2.h>

namespace Kortex::DownloadManager
{
	class DisplayModel: public KxDataView2::Model
	{
		private:
			BroadcastReciever m_BroadcastReciever;
			IDownloadManager& m_DownloadManager;
			std::list<DisplayModelNode> m_Nodes;

		private:
			void OnActivate(KxDataView2::Event& event);
			void OnContextMenu(KxDataView2::Event& event);

			void OnDownloadAdded(DownloadEvent& event);
			void OnDownloadRemoved(DownloadEvent& event);
			void OnDownloadProgress(DownloadEvent& event);

			void OnDownloadStarted(DownloadEvent& event);
			void OnDownloadCompleted(DownloadEvent& event);
			void OnDownloadFailed(DownloadEvent& event);

			void OnRefreshItems(DownloadEvent& event);

		private:
			template<class T> auto FindNode(T&& items, const DownloadItem& item) const
			{
				if (!item.IsHidden() || m_DownloadManager.ShouldShowHiddenDownloads())
				{
					return std::find_if(items.begin(), items.end(), [&item](auto&& node)
					{
						return &node.m_Item == &item;
					});
				}
				return items.end();
			}

			DownloadItem* GetItem(KxDataView2::Node* node)
			{
				DisplayModelNode* displayNode = static_cast<DisplayModelNode*>(node);
				return displayNode ? &displayNode->m_Item : nullptr;
			}
			auto GetNode(const DownloadItem& item) const
			{
				return FindNode(m_Nodes, item);
			}
			auto GetNode(DownloadItem& item)
			{
				return FindNode(m_Nodes, item);
			}

			DisplayModelNode* AddNode(DownloadItem& item);
			bool RemoveNode(DownloadItem& item);

			void RemoveAll(bool installedOnly = false);
			void SetAllHidden(bool isHidden, bool installedOnly = false);
			void Install(DownloadItem& item);

		public:
			DisplayModel();

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();
	};
}
