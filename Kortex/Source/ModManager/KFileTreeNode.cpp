#include "stdafx.h"
#include "KFileTreeNode.h"
#include "KModEntry.h"
#include "KModManager.h"

KFileTreeNode* KFileTreeNode::NavigateToElement(const KFileTreeNode& rootNode, const wxString& relativePath, KxFileSearchType type)
{
	if (rootNode.HasChildren() && !relativePath.IsEmpty())
	{
		wxString relativePathL = KxString::ToLower(relativePath);
		relativePathL.Replace("/", "\\");

		auto Recurse = [type](const KFileTreeNode& rootNode, const wxString& folderName) -> KFileTreeNode*
		{
			for (const KFileTreeNode& node: rootNode.GetChildren())
			{
				if (folderName == KxString::ToLower(node.GetName()))
				{
					return const_cast<KFileTreeNode*>(&node);
				}
			}
			return NULL;
		};

		KFileTreeNode* finalNode = NULL;
		for (const wxString& folderName: KxString::Split(relativePathL, "\\"))
		{
			finalNode = Recurse(finalNode ? *finalNode : rootNode, folderName);
			if (finalNode == NULL)
			{
				break;
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
