#include "stdafx.h"
#include "KModManagerDispatcher.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KFileTreeNode.h"
#include "KVariablesDatabase.h"
#include "KVirtualGameFolderWorkspace.h"
#include "UI/KMainWindow.h"
#include "KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileFinder.h>

namespace
{
	struct FinderHashComparator
	{
		bool operator()(const wxString& lhs, const wxString& rhs) const
		{
			return KxComparator::IsEqual(lhs, rhs, true);
		}
	};
	struct FinderHashHasher
	{
		// From Boost
		template<class T> static void hash_combine(size_t& seed, const T& v)
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		size_t operator()(const wxString& value) const
		{
			size_t hashValue = 0;
			for (const wxUniChar& c: value)
			{
				hash_combine(hashValue, KxString::ToLower(c).GetValue());
			}
			return hashValue;
		}
	};
	using FinderHash = std::unordered_map<wxString, size_t, FinderHashHasher, FinderHashComparator>;

	void FindFilesInTree(KFileTreeNode::CRefVector& nodes,
						 const KFileTreeNode& rootNode,
						 const wxString& filter,
						 KxFileSearchType type,
						 bool recurse,
						 bool activeOnly = false
	)
	{
		auto ScanTree = [&nodes, &filter, type, recurse, activeOnly](KFileTreeNode::CRefVector& directories, const KFileTreeNode& folderNode)
		{
			for (const KFileTreeNode& node: folderNode.GetChildren())
			{
				if ((activeOnly && node.GetMod().IsEnabled() || !activeOnly))
				{
					if (recurse && node.IsDirectory())
					{
						directories.push_back(&node);
					}

					if (node.GetItem().IsElementType(type) && KAux::CheckSearchMask(filter, node.GetName()))
					{
						nodes.push_back(&node);
					}
				}
			}
		};

		// Top level
		KFileTreeNode::CRefVector directories;
		ScanTree(directories, rootNode);
		
		// Subdirectories
		while (!directories.empty())
		{
			KFileTreeNode::CRefVector roundDirectories;
			for (const KFileTreeNode* node: directories)
			{
				ScanTree(roundDirectories, *node);
			}
			directories = std::move(roundDirectories);
		}
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
KModEntry* KModManagerDispatcher::IterateOverModsEx(const ModsVector& mods, const IterationFunctor& functor, IterationOrder order, bool activeOnly) const
{
	switch (order)
	{
		case IterationOrder::Direct:
		{
			for (KModEntry* entry: mods)
			{
				if (CheckConditionsAndCallFunctor(functor, *entry, activeOnly))
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
				if (CheckConditionsAndCallFunctor(functor, **it, activeOnly))
				{
					return *it;
				}
			}
			break;
		}
	};
	return NULL;
}
bool KModManagerDispatcher::CheckConditionsAndCallFunctor(const IterationFunctor& functor, const KModEntry& modEntry, bool activeOnly) const
{
	return modEntry.IsInstalled() && (activeOnly && modEntry.IsEnabled() || !activeOnly) && functor(modEntry);
}

void KModManagerDispatcher::BuildTreeBranch(const ModsVector& mods, KFileTreeNode::Vector& children, const KFileTreeNode* rootNode, KFileTreeNode::RefVector& directories)
{
	FinderHash hash;

	// Iterate manually, without using 'IterateOverModsEx'
	for (auto it = mods.rbegin(); it != mods.rend(); ++it)
	{
		const KModEntry& modEntry = **it;
		if (modEntry.IsInstalled())
		{
			// If we have root node, look for files in real file tree
			// Otherwise use mod's tree root
			const KFileTreeNode* searchNode = &modEntry.GetFileTree();
			if (rootNode)
			{
				searchNode = KFileTreeNode::NavigateToFolder(modEntry.GetFileTree(), rootNode->GetRelativePath());
			}

			if (searchNode)
			{
				// Not enough, but at least something
				children.reserve(searchNode->GetChildrenCount());

				for (const KFileTreeNode& node: searchNode->GetChildren())
				{
					auto hashIt = hash.try_emplace(node.GetName(), 0);
					if (hashIt.second)
					{
						KFileTreeNode& newNode = children.emplace_back(KFileTreeNode(modEntry, node.GetItem(), rootNode));
						
						// Save index to new node to add alternatives to it later
						// I'd use pointer, but it can be invalidated on reallocation
						hashIt.first->second = children.size() - 1;
					}
					else
					{
						const size_t index = hashIt.first->second;
						children[index].GetAlternatives().push_back(KFileTreeNode(modEntry, node.GetItem(), rootNode));
					}
				}
			}
		}
	}

	// Fill directories array
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

void KModManagerDispatcher::OnVirtualTreeInvalidated(KEvent& event)
{
	m_VirtualTreeNeedsRefresh = true;
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

	KModEvent(KEVT_MOD_VIRTUAL_TREE_INVALIDATED).Send();
}
void KModManagerDispatcher::InvalidateVirtualTree()
{
	m_VirtualTreeNeedsRefresh = true;
}

const KFileTreeNode& KModManagerDispatcher::GetVirtualTree() const
{
	RebuildTreeIfNeeded();
	return m_VirtualTree;
}

KModEntry* KModManagerDispatcher::IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget, bool activeOnly) const
{
	RebuildTreeIfNeeded();

	return IterateOverModsEx(KModManager::Get().GetAllEntries(includeWriteTarget), functor, order, activeOnly);
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

const KFileTreeNode* KModManagerDispatcher::BackTrackFullPath(const wxString& fullPath) const
{
	return m_VirtualTree.WalkTree([&fullPath](const KFileTreeNode& node)
	{
		return KxComparator::IsEqual(node.GetFullPath(), fullPath);
	});
}

KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const wxString& relativePath, const wxString& filter, KxFileSearchType type, bool recurse, bool activeOnly) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	const KFileTreeNode* folderNode = KFileTreeNode::NavigateToFolder(m_VirtualTree, relativePath);
	if (folderNode)
	{
		FindFilesInTree(nodes, *folderNode, filter, type, recurse, activeOnly);
	}
	return nodes;
}
KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const KFileTreeNode& rootNode, const wxString& filter, KxFileSearchType type, bool recurse, bool activeOnly) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, rootNode, filter, type, recurse, activeOnly);

	return nodes;
}
KFileTreeNode::CRefVector KModManagerDispatcher::FindFiles(const KModEntry& modEntry, const wxString& filter, KxFileSearchType type, bool recurse) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, modEntry.GetFileTree(), filter, type, recurse);

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
	KEvent::Bind(KEVT_MOD_INSTALLED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_UNINSTALLED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MODS_REORDERED, &KModManagerDispatcher::OnVirtualTreeInvalidated, this);
}
