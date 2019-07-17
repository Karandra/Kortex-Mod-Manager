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
			template<class T> static auto FindNode(T&& items, const DownloadItem& item)
			{
				return std::find_if(items.begin(), items.end(), [&item](auto&& node)
				{
					return &node.m_Item == &item;
				});
			}

		private:
			IDownloadManager& m_DownloadManager;
			std::list<DisplayModelNode> m_Nodes;

		private:
			void OnContextMenu(Event& event);

			void OnDownloadAdded(DownloadEvent& event);
			void OnDownloadRemoved(DownloadEvent& event);
			void OnDownloadProgress(DownloadEvent& event);

			void OnDownloadStarted(DownloadEvent& event);
			void OnDownloadCompleted(DownloadEvent& event);
			void OnDownloadFailed(DownloadEvent& event);

		private:
			DownloadItem* GetItem(Node* node)
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

			DisplayModelNode& AddNode(DownloadItem& item)
			{
				DisplayModelNode& node = m_Nodes.emplace_back(item);
				GetView()->GetRootNode().AttachChild(node);
				node.OnAttachNode();

				return node;
			}
			bool RemoveNode(DownloadItem& item)
			{
				if (auto node = GetNode(item); node != m_Nodes.end())
				{
					GetView()->GetRootNode().DetachChild(*node);
					m_Nodes.erase(node);
					return true;
				}
				return false;
			}

			void RemoveAll(bool installedOnly = false);
			void SetAllHidden(bool isHidden, bool installedOnly = false);
			void Install(DownloadItem& item);

		public:
			DisplayModel();
			~DisplayModel();

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();
	};
}
