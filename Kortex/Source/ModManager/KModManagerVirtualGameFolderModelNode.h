#pragma once
#include "stdafx.h"
#include <KxFramework/KxFileFinder.h>
class KModEntry;
class KFileTreeNode;
class KModManagerVirtualGameFolderModel;

namespace KModManagerVirtualGameFolderModelNS
{
	class ModelNode
	{
		friend class KModManagerVirtualGameFolderModel;

		public:
			using Vector = std::vector<std::unique_ptr<ModelNode>>;

		private:
			Vector m_Children;
			ModelNode* m_ParentNode = NULL;

			const KFileTreeNode& m_FileNode;
			const KModEntry* m_ModEntry = NULL;

			bool m_IsVisited = false;

		private:
			bool IsVisited() const
			{
				return m_IsVisited;
			}
			void MarkVisited()
			{
				m_IsVisited = true;
			}

		public:
			ModelNode(ModelNode* parent, const KFileTreeNode& fileNode, const KModEntry* modEntry)
				:m_ParentNode(parent), m_FileNode(fileNode), m_ModEntry(modEntry)
			{
			}

		public:
			bool HasParent() const
			{
				return m_ParentNode != NULL;
			}
			const ModelNode* GetParent() const
			{
				return m_ParentNode;
			}
			ModelNode* GetParent()
			{
				return m_ParentNode;
			}

			size_t GetChildrenCount() const
			{
				return m_Children.size();
			}
			bool HasChildren() const;
			const Vector& GetChildren() const
			{
				return m_Children;
			}
			Vector& GetChildren()
			{
				return m_Children;
			}

			const KFileTreeNode& GetFileNode() const
			{
				return m_FileNode;
			}
			const KxFileFinderItem& GetFileItem() const;
			const KModEntry* GetMod() const
			{
				return m_ModEntry;
			}
	};
}
