#include "stdafx.h"
#include "KModManagerVirtualGameFolderModelNode.h"
#include "KFileTreeNode.h"

bool KModManagerVirtualGameFolderModelNS::ModelNode::HasChildren() const
{
	if (m_FileNode.IsDirectory())
	{
		return !m_Children.empty() || m_FileNode.HasChildren();
	}
	return false;
}

const KxFileFinderItem& KModManagerVirtualGameFolderModelNS::ModelNode::GetFileItem() const
{
	return m_FileNode.GetItem();
}
