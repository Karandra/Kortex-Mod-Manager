#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxComparator.h>

namespace
{
	bool IsRequestToRootNode(const wxString& relativePath)
	{
		return relativePath.IsEmpty() || relativePath == wxS('\\') || relativePath == wxS('/') || relativePath == wxS('.') || relativePath == wxS("..");
	}
	template<class Functor> void IterateOverPath(const wxString& relativePath, const Functor& functor)
	{
		// This ugly construction is faster than
		// for (const wxString& folderName: KxString::Split(relativePath, wxS("\\")))
		// So using it.
		const constexpr wxChar separator = wxS('\\');
		size_t pos = 0;
		size_t separatorPos = relativePath.find(separator);
		if (separatorPos == wxString::npos)
		{
			separatorPos = relativePath.length();
		}

		while (pos < relativePath.length() && separatorPos <= relativePath.length())
		{
			const std::wstring_view folderName(relativePath.wc_str() + pos, separatorPos - pos);
			if (functor(folderName))
			{
				return;
			}

			pos += folderName.length() + 1;
			separatorPos = relativePath.find(separator, pos);

			// No separator found, but this is not the last element
			if (separatorPos == wxString::npos && pos < relativePath.length())
			{
				separatorPos = relativePath.length();
			}
		}
	}

	struct FileNameHasher
	{
		// From Boost
		template<class T> static void hash_combine(size_t& seed, const T& v)
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
		}
		
		size_t operator()(const std::wstring_view& value) const
		{
			size_t hashValue = 0;
			for (wchar_t c: value)
			{
				hash_combine(hashValue, KxString::CharToLower(c));
			}
			return hashValue;
		}
	};
}

namespace Kortex
{
	const FileTreeNode* FileTreeNode::NavigateToElement(const FileTreeNode& rootNode, const wxString& relativePath, NavigateTo type)
	{
		if (type == NavigateTo::Folder && IsRequestToRootNode(relativePath))
		{
			return &rootNode;
		}

		if (rootNode.HasChildren())
		{
			auto ScanChildren = [](const FileTreeNode& rootNode, const std::wstring_view& folderName) -> const FileTreeNode*
			{
				const size_t hash = HashFileName(folderName);
				for (const FileTreeNode& node: rootNode.GetChildren())
				{
					if (hash == node.GetNameHash())
					{
						return &node;
					}
				}
				return nullptr;
			};

			const FileTreeNode* finalNode = nullptr;
			IterateOverPath(relativePath, [&ScanChildren, &finalNode, &rootNode](const std::wstring_view& folderName)
			{
				finalNode = ScanChildren(finalNode ? *finalNode : rootNode, folderName);
				return finalNode == nullptr;
			});

			if (finalNode == nullptr || (type == NavigateTo::Folder && !finalNode->IsDirectory()) || type == NavigateTo::File && !finalNode->IsFile())
			{
				return nullptr;
			}
			return finalNode;
		}
		return nullptr;
	}

	size_t FileTreeNode::HashFileName(const std::wstring_view& name)
	{
		return FileNameHasher()(name);
	}

	const FileTreeNode* FileTreeNode::WalkTree(const TreeWalker& functor) const
	{
		std::function<const FileTreeNode*(const FileTreeNode::Vector&)> Recurse;
		Recurse = [&Recurse, &functor](const FileTreeNode::Vector& children) -> const FileTreeNode*
		{
			for (const FileTreeNode& node: children)
			{
				if (!functor(node))
				{
					return &node;
				}

				if (node.HasChildren())
				{
					Recurse(node.GetChildren());
				}
			}
			return nullptr;
		};
		return Recurse(m_Children);
	}
	const FileTreeNode* FileTreeNode::WalkToRoot(const TreeWalker& functor) const
	{
		const FileTreeNode* node = this;
		while (node && !node->IsRootNode() && node->IsOK())
		{
			if (functor(*node))
			{
				node = node->GetParent();
			}
			else
			{
				break;
			}
		}
		return node;
	}

	bool FileTreeNode::HasAlternativesFromActiveMods() const
	{
		if (!m_Alternatives.empty())
		{
			for (const FileTreeNode& node: m_Alternatives)
			{
				if (!node.GetMod().IsActive())
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	wxString FileTreeNode::GetRelativePath() const
	{
		wxString path = m_Item.GetFullPath();
		if (path.Replace(m_Mod->GetModFilesDir(), wxEmptyString, false) == 1 && !path.IsEmpty() && path[0] == wxS('\\'))
		{
			// Remove leading slash
			path.Remove(0, 1);
		}
		return path;
	}
}
