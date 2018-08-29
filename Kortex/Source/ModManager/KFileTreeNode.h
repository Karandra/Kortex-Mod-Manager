#pragma once
#include "stdafx.h"
#include <KxFramework/KxFileFinder.h>

class KFileTreeNode
{
	public:
		using FilesVector = std::vector<KxFileFinderItem>;
		using Vector = std::vector<KFileTreeNode>;

	private:
		static KFileTreeNode* NavigateToElement(const KFileTreeNode& rootNode, const wxString& relativePath, KxFileSearchType type);

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

	public:
		KFileTreeNode() = default;
		KFileTreeNode(const KxFileFinderItem& item, KFileTreeNode* parent = NULL)
			:m_Item(item), m_Parent(parent)
		{
		}

	public:
		bool IsRootNode() const
		{
			return m_Parent == NULL && !m_Item.IsOK();
		}
		void MakeNull()
		{
			*this = KFileTreeNode();
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
