#include "stdafx.h"
#include "KModManagerDispatcher.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KFileTreeNode.h"
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileFinder.h>

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
void KModManagerDispatcher::FindFilesInTree(KFileTreeNode::CRefVector& nodes,
											FinderHash* hash,
											const KFileTreeNode& rootNode,
											const wxString& filter,
											KxFileSearchType type,
											bool recurse,
											const wxString& absolutePath
) const
{
	std::function<void(const KFileTreeNode&)> ScanTree = [&ScanTree, &nodes, &filter, type, hash, recurse, &absolutePath](const KFileTreeNode& folderNode)
	{
		for (const KFileTreeNode& node: folderNode.GetChildren())
		{
			if (node.GetName().Matches(filter))
			{
				if (recurse && node.IsDirectory())
				{
					ScanTree(node);
				}

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
	};
	ScanTree(rootNode);
}

KModEntry* KModManagerDispatcher::IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget, bool activeOnly) const
{
	KModEntryArray entries = KModManager::Get().GetAllEntries(includeWriteTarget);
	switch (order)
	{
		case IterationOrder::Direct:
		{
			for (KModEntry* entry: entries)
			{
				if ((activeOnly && entry->IsEnabled() || !activeOnly) && functor(*entry))
				{
					return entry;
				}
			}
			break;
		}
		case IterationOrder::Reversed:
		{
			for (auto it = entries.rbegin(); it != entries.rend(); ++it)
			{
				if ((activeOnly && (*it)->IsEnabled() || !activeOnly) && functor(**it))
				{
					return *it;
				}
			}
			break;
		}
	};
	return NULL;
}

wxString KModManagerDispatcher::GetTargetPath(const wxString& relativePath, KModEntry** owningMod) const
{
	// This is absolute path, return it as is
	if (relativePath.Length() >= 2 && relativePath[1] == wxT(':'))
	{
		KxUtility::SetIfNotNull(owningMod, nullptr);
		return relativePath;
	}

	wxString outPath;
	auto CheckIn = [&outPath, &relativePath](const KModEntry& modEntry)
	{
		if (modEntry.IsEnabled())
		{
			KFileTreeNode* node = KFileTreeNode::NavigateToAny(modEntry.GetFileTree(), relativePath);
			if (node)
			{
				outPath = node->GetFullPath();
				return true;
			}
		}
		return false;
	};
	
	KModEntry* modEntry = IterateOverMods(CheckIn, IterationOrder::Reversed);
	KxUtility::SetIfNotNull(owningMod, modEntry);

	// Fallback to write target
	if (outPath.IsEmpty())
	{
		return KModManager::Get().GetModEntry_WriteTarget()->GetLocation(KMM_LOCATION_MOD_FILES) + '\\' + relativePath;
	}
	return outPath;
}

KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const KFileTreeNode& rootNode, const wxString& filter, KxFileSearchType type, bool recurse, FinderHash* hash) const
{
	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, hash, rootNode, filter, type, recurse, rootNode.GetMod().GetLocation(KMM_LOCATION_MOD_FILES));

	return nodes;
}
KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const KModEntry& modEntry, const wxString& filter, KxFileSearchType type, bool recurse, FinderHash* hash) const
{
	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, hash, modEntry.GetFileTree(), filter, type, recurse, modEntry.GetLocation(KMM_LOCATION_MOD_FILES));

	return nodes;
}
KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const wxString& relativePath, const wxString& filter, KxFileSearchType type, bool recurse, FinderHash* hash) const
{
	FinderHash localHash;
	FinderHash* activeHash = hash ? hash : &localHash;

	KFileTreeNode::CRefVector nodes;
	auto FindIn = [this, &relativePath, &nodes, activeHash, &filter, type, recurse](const KModEntry& modEntry)
	{
		if (modEntry.IsEnabled())
		{
			KFileTreeNode* folderNode = KFileTreeNode::NavigateToFolder(modEntry.GetFileTree(), relativePath);
			if (folderNode)
			{
				FindFilesInTree(nodes, activeHash, *folderNode, filter, type, recurse, modEntry.GetLocation(KMM_LOCATION_MOD_FILES));
			}
		}
		return false;
	};

	IterateOverMods(FindIn, IterationOrder::Reversed);
	return nodes;
}

KModManagerDispatcher::CollisionVector KModManagerDispatcher::FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const
{
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
