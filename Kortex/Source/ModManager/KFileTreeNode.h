#pragma once
#include "stdafx.h"
#include <KxFramework/KxFileFinder.h>
class KModEntry;
class KDispatcher;

class KFileTreeNode
{
	friend class KDispatcher;
	friend class KModEntry;

	public:
		using Vector = std::vector<KFileTreeNode>;
		using RefVector = std::vector<KFileTreeNode*>;
		using CRefVector = std::vector<const KFileTreeNode*>;

		using TreeWalker = std::function<bool(const KFileTreeNode&)>;

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

		static size_t HashFileName(const std::wstring_view& name);
		static size_t HashFileName(const wxString& name)
		{
			return HashFileName(std::wstring_view(name.wc_str(), name.length()));
		}

	private:
		Vector m_Children;
		Vector m_Alternatives;
		KxFileItem m_Item;
		const KFileTreeNode* m_Parent = NULL;
		const KModEntry* m_Mod = NULL;
		size_t m_NameHash = 0;

	private:
		void MakeNull()
		{
			*this = KFileTreeNode();
		}
		void SetParent(const KFileTreeNode* parent)
		{
			m_Parent = parent;
		}

	public:
		KFileTreeNode() = default;
		KFileTreeNode(const KModEntry& modEntry, const KxFileItem& item, const KFileTreeNode* parent = NULL)
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

		const KFileTreeNode* WalkTree(const TreeWalker& functor) const;
		const KFileTreeNode* WalkToRoot(const TreeWalker& functor) const;

		void CopyBasicAttributes(const KFileTreeNode& other)
		{
			m_NameHash = other.m_NameHash;
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
		Vector& GetChildren()
		{
			return m_Children;
		}
		void ClearChildren()
		{
			m_Children.clear();
		}

		bool HasAlternatives() const
		{
			return !m_Alternatives.empty();
		}
		bool HasAlternativesFromActiveMods() const;
		size_t GetAlternativesCount() const
		{
			return m_Alternatives.size();
		}
		const Vector& GetAlternatives() const
		{
			return m_Alternatives;
		}
		Vector& GetAlternatives()
		{
			return m_Alternatives;
		}
		void ClearAlternatives()
		{
			m_Alternatives.clear();
		}

		const KxFileItem& GetItem() const
		{
			return m_Item;
		}
		KxFileItem& GetItem()
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

		size_t GetNameHash() const
		{
			return m_NameHash;
		}
		void ComputeHash()
		{
			m_NameHash = HashFileName(GetName());
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
