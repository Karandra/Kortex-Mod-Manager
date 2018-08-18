#include "stdafx.h"
#include "KFileTreeNode.h"

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
				if (node.GetItem().IsElementType(type) && folderName == KxString::ToLower(node.GetName()))
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
		return finalNode;
	}
	return NULL;
}
