#include "stdafx.h"
#include "KFileTreeNode.h"
#include "KModEntry.h"
#include "KModManager.h"
#include "KComparator.h"

const KFileTreeNode* KFileTreeNode::NavigateToElement(const KFileTreeNode& rootNode, const wxString& relativePath, KxFileSearchType type)
{
	if (type == KxFileSearchType::KxFS_FOLDER)
	{
		if (relativePath.IsEmpty() || *relativePath.begin() == wxS('\\') || *relativePath.begin() == wxS('/') || *relativePath.begin() == wxS('.'))
		{
			return rootNode.GetRootNode();
		}
	}

	if (rootNode.HasChildren())
	{
		auto ScanChildren = [](const KFileTreeNode& rootNode, const wxString& folderName) -> const KFileTreeNode*
		{
			for (const KFileTreeNode& node: rootNode.GetChildren())
			{
				if (KComparator::KEqual(folderName, node.GetName(), true))
				{
					return &node;
				}
			}
			return NULL;
		};

		const KFileTreeNode* finalNode = NULL;

		// This ugly construction is faster than
		// for (const wxString& folderName: KxString::Split(relativePath, wxS("\\")))
		// So using it.
		const wxChar separator = wxS('\\');
		size_t pos = 0;
		size_t separatorPos = relativePath.find(separator);
		if (separatorPos == wxString::npos)
		{
			separatorPos = relativePath.length();
		}

		while (pos < relativePath.length() && separatorPos <= relativePath.length())
		{
			const wxString folderName = relativePath.SubString(pos, separatorPos - 1);
			finalNode = ScanChildren(finalNode ? *finalNode : rootNode, folderName);
			if (finalNode == NULL)
			{
				break;
			}

			pos += folderName.length() + 1;
			separatorPos = relativePath.find(separator, pos);

			// No separator found, but this is not the last element
			if (separatorPos == wxString::npos && pos < relativePath.length())
			{
				separatorPos = relativePath.length();
			}
		}

		if (finalNode && finalNode->GetItem().IsElementType(type))
		{
			return finalNode;
		}
	}
	return NULL;
}

wxString KFileTreeNode::GetRelativePath() const
{
	wxString path = m_Item.GetFullPath();
	if (path.Replace(m_Mod->GetLocation(KMM_LOCATION_MOD_FILES), wxEmptyString, false) == 1 && !path.IsEmpty() && path[0] == wxS('\\'))
	{
		// Remove leading slash
		path.Remove(0, 1);
	}
	return path;
}
