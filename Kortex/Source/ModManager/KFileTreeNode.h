#pragma once
#include "stdafx.h"
#include <KxFramework/KxFileFinder.h>
class KModEntry;
class KModManagerDispatcher;

class KFileTreeNode
{
	friend class KModManagerDispatcher;
	friend class KModEntry;

	public:
		using Vector = std::vector<KFileTreeNode>;
		using RefVector = std::vector<KFileTreeNode*>;
		using CRefVector = std::vector<const KFileTreeNode*>;

	private:
		static const KFileTreeNode* NavigateToElement(const KFileTreeNode& rootNode, const wxString& relativePath, KxFileSearchType type);
		template<class T> static T* FindRootNode(T* thisNode)
		{
			T* node = thisNode;
			while (node && !node->IsRootNode() && node->IsOK())
			{
				node = node->GetParent();
			}
			return node;
		}
		template<class NodesVector, class NodesRefVector> static void RepackToRefVector(NodesVector& nodes, NodesRefVector& nodeRefs)
		{
			nodeRefs.reserve(nodes.size());
			for (auto& node: nodes)
			{
				nodeRefs.push_back(&node);
			}
		}

	public:
		static const KFileTreeNode* NavigateToFolder(const KFileTreeNode& rootNode, const wxString& relativePath)
		{
			return NavigateToElement(rootNode, relativePath, KxFileSearchType::KxFS_FOLDER);
		}
		static const KFileTreeNode* NavigateToFile(const KFileTreeNode& rootNode, const wxString& relativePath)
		{
			return NavigateToElement(rootNode, relativePath, KxFileSearchType::KxFS_FILE);
		}
		static const KFileTreeNode* NavigateToAny(const KFileTreeNode& rootNode, const wxString& relativePath)
		{
			return NavigateToElement(rootNode, relativePath, KxFileSearchType::KxFS_ALL);
		}

		static void ToRefVector(Vector& nodes, RefVector& refItems)
		{
			RepackToRefVector(nodes, refItems);
		}
		static void ToCRefVector(const Vector& nodes, CRefVector& refItems)
		{
			RepackToRefVector(nodes, refItems);
		}

	private:
		Vector m_Children;
		Vector m_Alternatives;
		KxFileFinderItem m_Item;
		const KFileTreeNode* m_Parent = NULL;
		const KModEntry* m_Mod = NULL;

	private:
		void MakeNull()
		{
			*this = KFileTreeNode();
		}
		void SetParent(const KFileTreeNode* parent)
		{
			m_Parent = parent;
		}

		Vector& GetChildren()
		{
			return m_Children;
		}
		void ClearChildren()
		{
			m_Children.clear();
		}
		
		Vector& GetAlternatives()
		{
			return m_Alternatives;
		}
		void ClearAlternatives()
		{
			m_Alternatives.clear();
		}

	public:
		KFileTreeNode() = default;
		KFileTreeNode(const KModEntry& modEntry, const KxFileFinderItem& item, const KFileTreeNode* parent = NULL)
			:m_Mod(&modEntry), m_Item(item), m_Parent(parent)
		{
		}

	public:
		bool IsOK() const
		{
			return m_Mod != NULL;
		}
		bool IsRootNode() const
		{
			return m_Parent == NULL && !m_Item.IsOK();
		}

		const KModEntry& GetMod() const
		{
			return *m_Mod;
		}

		bool HasChildren() const
		{
			return !m_Children.empty();
		}
		size_t GetChildrenCount() const
		{
			return m_Children.size();
		}
		const Vector& GetChildren() const
		{
			return m_Children;
		}
		
		bool HasAlternatives() const
		{
			return !m_Alternatives.empty();
		}
		size_t GetAlternativesCount() const
		{
			return m_Alternatives.size();
		}
		const Vector& GetAlternatives() const
		{
			return m_Alternatives;
		}
		
		const KxFileFinderItem& GetItem() const
		{
			return m_Item;
		}
		KxFileFinderItem& GetItem()
		{
			return m_Item;
		}

		bool HasParent() const
		{
			return m_Parent != NULL;
		}
		const KFileTreeNode* GetParent() const
		{
			return m_Parent;
		}
		const KFileTreeNode* GetRootNode() const
		{
			return FindRootNode(this);
		}

		const wxString& GetName() const
		{
			return m_Item.GetName();
		}
		const wxString& GetSource() const
		{
			return m_Item.GetSource();
		}
		wxString GetFullPath() const
		{
			return m_Item.GetFullPath();
		}
		wxString GetRelativePath() const;

		bool IsDirectory() const
		{
			return m_Item.IsDirectory();
		}
		bool IsFile() const
		{
			return m_Item.IsFile();
		}
		int64_t GetFileSize() const
		{
			return m_Item.GetFileSize();
		}
};
