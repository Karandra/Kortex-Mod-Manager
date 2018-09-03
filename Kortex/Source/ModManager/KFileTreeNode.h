#pragma once
#include "stdafx.h"
#include <KxFramework/KxFileFinder.h>
class KModEntry;

class KFileTreeNode
{
	public:
		using FilesVector = std::vector<KxFileFinderItem>;
		using Vector = std::vector<KFileTreeNode>;
		using RefVector = std::vector<KFileTreeNode*>;
		using CRefVector = std::vector<const KFileTreeNode*>;

	private:
		static KFileTreeNode* NavigateToElement(const KFileTreeNode& rootNode, const wxString& relativePath, KxFileSearchType type);
		template<class T> static T* FindRootNode(T* thisNode)
		{
			T* node = thisNode;
			while (node && !node->IsRootNode() && node->IsOK())
			{
				node = node->GetParent();
			}
			return node;
		}

	public:
		static KFileTreeNode* NavigateToFolder(const KFileTreeNode& rootNode, const wxString& relativePath)
		{
			return NavigateToElement(rootNode, relativePath, KxFileSearchType::KxFS_FOLDER);
		}
		static KFileTreeNode* NavigateToFile(const KFileTreeNode& rootNode, const wxString& relativePath)
		{
			return NavigateToElement(rootNode, relativePath, KxFileSearchType::KxFS_FILE);
		}
		static KFileTreeNode* NavigateToAny(const KFileTreeNode& rootNode, const wxString& relativePath)
		{
			return NavigateToElement(rootNode, relativePath, KxFileSearchType::KxFS_ALL);
		}

	private:
		KxFileFinderItem m_Item;
		Vector m_Children;
		KFileTreeNode* m_Parent = NULL;
		const KModEntry* m_Mod = NULL;

	public:
		KFileTreeNode() = default;
		KFileTreeNode(const KModEntry& modEntry, const KxFileFinderItem& item, KFileTreeNode* parent = NULL)
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
		void MakeNull()
		{
			*this = KFileTreeNode();
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
		Vector& GetChildren()
		{
			return m_Children;
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
		KFileTreeNode* GetParent()
		{
			return m_Parent;
		}
		
		const KFileTreeNode* GetRootNode() const
		{
			return FindRootNode(this);
		}
		KFileTreeNode* GetRootNode()
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
