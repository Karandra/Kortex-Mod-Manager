#include "stdafx.h"
#include "KModManagerDispatcher.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KFileTreeNode.h"
#include "KVariablesDatabase.h"
#include "KModManagerVirtualGameFolderWS.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileFinder.h>

namespace
{
	void FindFilesInTree(KFileTreeNode::CRefVector& nodes,
						 KModManagerDispatcher::FinderHash* hash,
						 const KFileTreeNode& rootNode,
						 const wxString& filter,
						 KxFileSearchType type,
						 bool recurse,
						 const wxString& absolutePath = wxEmptyString,
						 bool activeOnly = false
	)
	{
		std::function<void(const KFileTreeNode&)> ScanTree;
		ScanTree = [&ScanTree, &nodes, &filter, type, hash, recurse, &absolutePath, activeOnly](const KFileTreeNode& folderNode)
		{
			for (const KFileTreeNode& node: folderNode.GetChildren())
			{
				if ((activeOnly && node.GetMod().IsEnabled() || !activeOnly))
				{
					if (recurse && node.IsDirectory())
					{
						ScanTree(node);
					}

					if (KAux::CheckSearchMask(filter, node.GetName()))
					{
						if (node.GetItem().IsElementType(type))
						{
							if (hash)
							{
								wxString id = node.GetFullPath();
								if (!absolutePath.IsEmpty())
								{
									id.Replace(absolutePath, wxEmptyString, false);
								}
								KxString::MakeLower(id);

								if (hash->insert(id).second)
								{
									nodes.push_back(&node);
								}
							}
							else
							{
								nodes.push_back(&node);
							}
						}
					}
				}
			}
		};
		ScanTree(rootNode);
	}
}

wxString KMMDispatcherCollision::GetLocalizedCollisionName(KMMDispatcherCollisionType type)
{
	switch (type)
	{
		case KMM_DCT_NONE:
		{
			return T(KxID_NONE);
		}
		case KMM_DCT_UNKNOWN:
		{
			return T("ModExplorer.Collision.Unknown");
		}
		case KMM_DCT_OVERWRITTEN:
		{
			return T("ModExplorer.Collision.Overwritten");
		}
		case KMM_DCT_OVERWRITES:
		{
			return T("ModExplorer.Collision.Owerwrites");
		}
	};
	return wxEmptyString;
}

//////////////////////////////////////////////////////////////////////////
KModEntry* KModManagerDispatcher::IterateOverModsEx(const ModsVector& mods, const IterationFunctor& functor, IterationOrder order, bool activeOnly, bool realMode) const
{
	switch (order)
	{
		case IterationOrder::Direct:
		{
			for (KModEntry* entry: mods)
			{
				if (CheckConditionsAndCallFunctor(functor, *entry, activeOnly, realMode))
				{
					return entry;
				}
			}
			break;
		}
		case IterationOrder::Reversed:
		{
			for (auto it = mods.rbegin(); it != mods.rend(); ++it)
			{
				if (CheckConditionsAndCallFunctor(functor, **it, activeOnly, realMode))
				{
					return *it;
				}
			}
			break;
		}
	};
	return NULL;
}
bool KModManagerDispatcher::CheckConditionsAndCallFunctor(const IterationFunctor& functor, const KModEntry& modEntry, bool activeOnly, bool realMode) const
{
	return (realMode ? modEntry.IsInstalledReal() : modEntry.IsInstalled()) && (activeOnly && modEntry.IsEnabled() || !activeOnly) && functor(modEntry);
}

void KModManagerDispatcher::BuildTreeBranch(const ModsVector& mods, KFileTreeNode::Vector& children, const KFileTreeNode* rootNode, KFileTreeNode::RefVector& directories)
{
	FinderHash hash;
	const bool isRealMode = rootNode == NULL;

	IterateOverModsEx(mods, [&children, &hash, rootNode, &directories](const KModEntry& modEntry)
	{
		KFileTreeNode::CRefVector fileNodes;
		if (rootNode)
		{
			// If we have root node, look for files in real file tree
			const KFileTreeNode* folderNode = KFileTreeNode::NavigateToFolder(modEntry.GetFileTree(), rootNode->GetRelativePath());
			if (folderNode)
			{
				FindFilesInTree(fileNodes, &hash, *folderNode, KxFile::NullFilter, KxFS_ALL, false, modEntry.GetLocation(KMM_LOCATION_MOD_FILES));
			}
		}
		else
		{
			// Here we have real tree anyway
			FindFilesInTree(fileNodes, &hash, modEntry.GetFileTree(), KxFile::NullFilter, KxFS_ALL, false, modEntry.GetLocation(KMM_LOCATION_MOD_FILES));
		}

		for (const KFileTreeNode* node: fileNodes)
		{
			children.push_back(KFileTreeNode(modEntry, node->GetItem(), rootNode));
		}
		return false;
	}, IterationOrder::Reversed, false, isRealMode);

	for (KFileTreeNode& node: children)
	{
		if (node.IsDirectory())
		{
			directories.push_back(&node);
		}
	}
}
void KModManagerDispatcher::RebuildTreeIfNeeded() const
{
	if (m_VirtualTreeNeedsRefresh)
	{
		const_cast<KModManagerDispatcher*>(this)->m_VirtualTreeNeedsRefresh = false;
		const_cast<KModManagerDispatcher*>(this)->UpdateVirtualTree();
	}
}

void KModManagerDispatcher::OnVirtualTreeInvalidated(KModEvent& event)
{
	m_VirtualTreeNeedsRefresh = true;

	KModManagerVirtualGameFolderWS* workspace = KModManagerVirtualGameFolderWS::GetInstance();
	if (workspace && workspace->IsWorkspaceVisible())
	{
		RebuildTreeIfNeeded();
		workspace->ScheduleReload();
	}
}

void KModManagerDispatcher::UpdateVirtualTree()
{
	m_VirtualTree.ClearChildren();

	// Build top level
	ModsVector mods = KModManager::Get().GetAllEntries(true);
	KFileTreeNode::RefVector directories;
	BuildTreeBranch(mods, m_VirtualTree.GetChildren(), NULL, directories);

	// Build subdirectories
	while (!directories.empty())
	{
		KFileTreeNode::RefVector roundDirectories;
		for (KFileTreeNode* node: directories)
		{
			BuildTreeBranch(mods, node->GetChildren(), node, roundDirectories);
		}
		directories = std::move(roundDirectories);
	}
}

KModEntry* KModManagerDispatcher::IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget, bool activeOnly) const
{
	RebuildTreeIfNeeded();

	return IterateOverModsEx(KModManager::Get().GetAllEntries(includeWriteTarget), functor, order, activeOnly, false);
}

const KFileTreeNode* KModManagerDispatcher::ResolveLocation(const wxString& relativePath) const
{
	RebuildTreeIfNeeded();

	return KFileTreeNode::NavigateToAny(m_VirtualTree, relativePath);
}
wxString KModManagerDispatcher::ResolveLocationPath(const wxString& relativePath, const KModEntry** owningMod) const
{
	// This is an absolute path, return it as is.
	if (relativePath.Length() >= 2 && relativePath[1] == wxT(':'))
	{
		KxUtility::SetIfNotNull(owningMod, nullptr);
		return relativePath;
	}

	if (const KFileTreeNode* node = ResolveLocation(relativePath))
	{
		KxUtility::SetIfNotNull(owningMod, &node->GetMod());
		return node->GetFullPath();
	}

	// Fallback to write target
	KxUtility::SetIfNotNull(owningMod, nullptr);
	return KModManager::Get().GetModEntry_WriteTarget()->GetLocation(KMM_LOCATION_MOD_FILES) + wxS('\\') + relativePath;
}

KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const KFileTreeNode& rootNode, const wxString& filter, KxFileSearchType type, bool recurse, FinderHash* hash) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, hash, rootNode, filter, type, recurse, rootNode.GetMod().GetLocation(KMM_LOCATION_MOD_FILES));

	return nodes;
}
KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const KModEntry& modEntry, const wxString& filter, KxFileSearchType type, bool recurse, FinderHash* hash) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, hash, modEntry.GetFileTree(), filter, type, recurse, modEntry.GetLocation(KMM_LOCATION_MOD_FILES));

	return nodes;
}
KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const wxString& relativePath, const wxString& filter, KxFileSearchType type, bool recurse, FinderHash* hash) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	const KFileTreeNode* folderNode = KFileTreeNode::NavigateToFolder(m_VirtualTree, relativePath);
	if (folderNode)
	{
		FindFilesInTree(nodes, NULL, *folderNode, filter, type, recurse, wxEmptyString, true);
	}
	return nodes;
}

KModManagerDispatcher::CollisionVector KModManagerDispatcher::FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const
{
	RebuildTreeIfNeeded();

	CollisionVector collisions;
	KMMDispatcherCollisionType type = KMM_DCT_OVERWRITTEN;
	auto CheckMod = [&scannedMod, &collisions, &type, &relativePath](const KModEntry& modEntry)
	{
		// Flip collision type after this mod is found
		if (&modEntry == &scannedMod)
		{
			type = KMM_DCT_OVERWRITES;

			// Don't count scanned mod itself
			return false;
		}

		// If this mod is enabled and it have such file - add collision info for it.
		if (modEntry.IsEnabled())
		{
			const KFileTreeNode* node = KFileTreeNode::NavigateToAny(modEntry.GetFileTree(), relativePath);
			if (node && node->IsFile())
			{
				collisions.emplace_back(&modEntry, type);
			}
		}
		return false;
	};
	
	IterateOverMods(CheckMod, IterationOrder::Reversed);
	std::reverse(collisions.begin(), collisions.end());
	return collisions;
}

KModManagerDispatcher::KModManagerDispatcher()
{
	KEvent::Bind(KEVT_MOD_FILES_CHANGED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	//KEvent::Bind(KEVT_MOD_TOGGLED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_INSTALLED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_UNINSTALLED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_REORDERED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MODS_REORDERED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
}
