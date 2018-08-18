#pragma once
#include "stdafx.h"
#include <KxFramework/KxFileFinder.h>
#include "KModManager.h"
#include "KModManagerDispatcher.h"

class KMFEModelNode
{
	public:
		using NodeVector = std::vector<KMFEModelNode>;
		using NodeRefVector = std::vector<KMFEModelNode*>;
		using CollisionVector = KModManagerDispatcher::CollisionVector;

	private:
		KxFileFinderItem m_FileItem;
		const KMFEModelNode* m_ParentNode = NULL;
		NodeVector m_Children;

		bool m_CollisionsChecked = false;
		CollisionVector m_Collisions;

	public:
		KMFEModelNode(const KxFileFinderItem& item)
			:m_FileItem(item)
		{
		}

	public:
		bool IsOK() const
		{
			return m_FileItem.IsOK();
		}
		bool IsDirectory() const
		{
			return m_FileItem.IsDirectory();
		}
		bool IsFile() const
		{
			return m_FileItem.IsFile();
		}

		KxFileFinderItem* GetFSItem()
		{
			return IsOK() ? &m_FileItem : NULL;
		}
		KxFileFinderItem* GetFile()
		{
			return IsFile() ? &m_FileItem : NULL;
		}
		KxFileFinderItem* GetDirectory()
		{
			return IsDirectory() ? &m_FileItem : NULL;
		}

		bool HasParentNode() const
		{
			return m_ParentNode != NULL;
		}
		const KMFEModelNode* GetParentNode() const
		{
			return m_ParentNode;
		}
		void SetParentNode(const KMFEModelNode* node)
		{
			m_ParentNode = node;
		}

		bool HasChildren() const
		{
			return !m_Children.empty() && IsDirectory();
		}
		size_t GetChildrenCount() const
		{
			return m_Children.size();
		}
		const NodeVector& GetChildren() const
		{
			return m_Children;
		}
		NodeVector& GetChildren()
		{
			return m_Children;
		}

		bool HasCollisions() const
		{
			return !m_Collisions.empty();
		}
		const CollisionVector& GetCollisions(const KModEntry* modEntry)
		{
			if (IsFile())
			{
				if (!m_CollisionsChecked)
				{
					wxString path = m_FileItem.GetFullPath().Remove(0, modEntry->GetLocation(KMM_LOCATION_MOD_FILES).Length());
					m_Collisions = KModManager::GetDispatcher().FindCollisions(modEntry, path);
					m_CollisionsChecked = true;
				}
			}
			return m_Collisions;
		}
};
