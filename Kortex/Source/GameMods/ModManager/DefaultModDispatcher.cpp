#include "stdafx.h"
#include "DefaultModDispatcher.h"
#include "GameMods/IModManager.h"
#include "GameMods/FileTreeNode.h"
#include <Kortex/Events.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileFinder.h>
#include <chrono>
#include <execution>

namespace
{
	using namespace Kortex;
	using namespace Kortex::ModManager;

	int64_t GetClockTime()
	{
		using namespace std::chrono;
		return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	};

	struct FinderHashComparator
	{
		bool operator()(const wxString& lhs, const wxString& rhs) const
		{
			return KxComparator::IsEqual(lhs, rhs, true);
		}
	};
	struct FinderHashHasher
	{
		size_t operator()(const wxString& value) const
		{
			return FileTreeNode::HashFileName(value);
		}
	};
	using FinderHash = std::unordered_map<wxString, size_t, FinderHashHasher, FinderHashComparator>;

	void FindFilesInTree(FileTreeNode::CRefVector& nodes, const FileTreeNode& rootNode, const IModDispatcher::FilterFunctor& filter, bool recurse)
	{
		auto ScanSubTree = [&nodes, &filter, recurse](FileTreeNode::CRefVector& directories, const FileTreeNode& folderNode)
		{
			for (const FileTreeNode& node: folderNode.GetChildren())
			{
				// Add node if no filter is specified or filter accepts this node
				if (!filter || filter(node))
				{
					nodes.push_back(&node);
				}

				// Add directory to scan on next round
				if (recurse && node.IsDirectory())
				{
					directories.push_back(&node);
				}
			}
		};

		// Top level
		FileTreeNode::CRefVector directories;
		ScanSubTree(directories, rootNode);
		
		// Subdirectories
		while (!directories.empty())
		{
			FileTreeNode::CRefVector roundDirectories;
			for (const FileTreeNode* node: directories)
			{
				ScanSubTree(roundDirectories, *node);
			}
			directories = std::move(roundDirectories);
		}
	}
	void BuildTreeBranch(const IGameMod::RefVector& mods, FileTreeNode::Vector& children, const FileTreeNode* rootNode, FileTreeNode::RefVector& directories)
	{
		std::unordered_map<size_t, size_t> hash;
		hash.reserve(mods.size());
		const wxString rootPath = rootNode ? rootNode->GetRelativePath() : wxEmptyString;

		// Iterate manually, without using 'IterateOverModsEx'
		for (auto it = mods.rbegin(); it != mods.rend(); ++it)
		{
			const IGameMod& currentMod = **it;
			if (currentMod.IsInstalled())
			{
				// If we have root node, look for files in real file tree
				// Otherwise use mod's tree root
				const FileTreeNode* searchNode = &currentMod.GetFileTree();
				if (rootNode)
				{
					searchNode = FileTreeNode::NavigateToFolder(currentMod.GetFileTree(), rootPath);
				}

				if (searchNode)
				{
					// Not enough, but at least something
					children.reserve(searchNode->GetChildrenCount());

					for (const FileTreeNode& node: searchNode->GetChildren())
					{
						auto hashIt = hash.try_emplace(node.GetNameHash(), (size_t)-1);
						if (hashIt.second)
						{
							FileTreeNode& newNode = children.emplace_back(currentMod, node.GetItem(), rootNode);
							newNode.CopyBasicAttributes(node);

							// Save index to new node to add alternatives to it later.
							// I'd use pointer, but it can be invalidated on reallocation.
							hashIt.first->second = children.size() - 1;
						}
						else
						{
							const size_t index = hashIt.first->second;
							FileTreeNode& newAlternative = children[index].GetAlternatives().emplace_back(currentMod, node.GetItem(), rootNode);
							newAlternative.CopyBasicAttributes(node);
						}
					}
				}
			}
		}

		// Fill directories array
		for (FileTreeNode& node: children)
		{
			if (node.IsDirectory())
			{
				directories.push_back(&node);
			}
		}
	}
}

namespace Kortex::ModManager
{
	wxString KDispatcherCollision::GetLocalizedCollisionName(KMMDispatcherCollisionType type)
	{
		switch (type)
		{
			case KMM_DCT_NONE:
			{
				return KTr(KxID_NONE);
			}
			case KMM_DCT_UNKNOWN:
			{
				return KTr("ModExplorer.Collision.Unknown");
			}
			case KMM_DCT_OVERWRITTEN:
			{
				return KTr("ModExplorer.Collision.Overwritten");
			}
			case KMM_DCT_OVERWRITES:
			{
				return KTr("ModExplorer.Collision.Owerwrites");
			}
		};
		return wxEmptyString;
	}
}

namespace Kortex::ModManager
{
	bool DispatcherSearcher::operator()(const FileTreeNode& node) const
	{
		return node.GetItem().IsElementType(m_ElementType) &&
			(!m_ActiveOnly || (m_ActiveOnly && node.GetMod().IsActive())) &&
			KxComparator::Matches(node.GetName(), m_Filter, true);
	}
}

namespace Kortex::ModManager
{
	void DefaultModDispatcher::RebuildTreeIfNeeded() const
	{
		if (m_VirtualTreeInvalidated)
		{
			m_VirtualTreeInvalidated = false;
			const_cast<DefaultModDispatcher*>(this)->UpdateVirtualTree();
		}
	}

	void DefaultModDispatcher::OnVirtualTreeInvalidated(IEvent& event)
	{
		InvalidateVirtualTree();
	}

	void DefaultModDispatcher::UpdateVirtualTree()
	{
		const IGameMod::RefVector mods = IModManager::GetInstance()->GetAllMods(true);
		m_VirtualTree.ClearChildren();

		constexpr const bool useRecursive = true;
		const int64_t t1 = GetClockTime();

		if constexpr (useRecursive)
		{
			// Recursive (parallel)
			// Build top level
			FileTreeNode::RefVector directories;
			BuildTreeBranch(mods, m_VirtualTree.GetChildren(), nullptr, directories);

			// Build subdirectories
			std::function<void(FileTreeNode::RefVector&&)> Execute;
			Execute = [this, &Execute, &mods](FileTreeNode::RefVector&& directories)
			{
				std::for_each(std::execution::par_unseq, directories.begin(), directories.end(), [this, &Execute, &mods](FileTreeNode* node)
				{
					FileTreeNode::RefVector directories;
					BuildTreeBranch(mods, node->GetChildren(), node, directories);
					Execute(std::move(directories));
				});
			};
			Execute(std::move(directories));
		}
		else
		{
			// Iterational (sequential)
			// Build top level
			FileTreeNode::RefVector directories;
			BuildTreeBranch(mods, m_VirtualTree.GetChildren(), nullptr, directories);

			// Build subdirectories
			while (!directories.empty())
			{
				FileTreeNode::RefVector roundDirectories;
				roundDirectories.reserve(directories.size());

				for (FileTreeNode* node: directories)
				{
					BuildTreeBranch(mods, node->GetChildren(), node, roundDirectories);
				}
				directories = std::move(roundDirectories);
			}
		}
		wxLogInfo("KDispatcher::UpdateVirtualTree: %lld", GetClockTime() - t1);
	}
	void DefaultModDispatcher::InvalidateVirtualTree()
	{
		m_VirtualTreeInvalidated = true;
		IEvent::MakeSend<ModEvent>(Events::ModVirtualTreeInvalidated);
	}

	const FileTreeNode& DefaultModDispatcher::GetVirtualTree() const
	{
		RebuildTreeIfNeeded();
		return m_VirtualTree;
	}

	const FileTreeNode* DefaultModDispatcher::ResolveLocation(const wxString& relativePath) const
	{
		RebuildTreeIfNeeded();
		return FileTreeNode::NavigateToAny(m_VirtualTree, relativePath);
	}
	wxString DefaultModDispatcher::ResolveLocationPath(const wxString& relativePath, const IGameMod** owningMod) const
	{
		// This is an absolute path, return it as is.
		if (relativePath.Length() >= 2 && relativePath[1] == wxT(':'))
		{
			KxUtility::SetIfNotNull(owningMod, nullptr);
			return relativePath;
		}

		if (const FileTreeNode* node = ResolveLocation(relativePath))
		{
			KxUtility::SetIfNotNull(owningMod, &node->GetMod());
			return node->GetFullPath();
		}

		// Fallback to write target
		KxUtility::SetIfNotNull(owningMod, nullptr);
		return IModManager::GetInstance()->GetOverwrites().GetModFilesDir() + wxS('\\') + relativePath;
	}
	const FileTreeNode* DefaultModDispatcher::BackTrackFullPath(const wxString& fullPath) const
	{
		RebuildTreeIfNeeded();
		return m_VirtualTree.WalkTree([&fullPath](const FileTreeNode& node)
		{
			return KxComparator::IsEqual(node.GetFullPath(), fullPath, true);
		});
	}

	FileTreeNode::CRefVector DefaultModDispatcher::Find(const wxString& relativePath, const FilterFunctor& filter, bool recurse) const
	{
		RebuildTreeIfNeeded();

		FileTreeNode::CRefVector nodes;
		const FileTreeNode* folderNode = FileTreeNode::NavigateToFolder(m_VirtualTree, relativePath);
		if (folderNode)
		{
			FindFilesInTree(nodes, *folderNode, filter, recurse);
		}
		return nodes;
	}
	FileTreeNode::CRefVector DefaultModDispatcher::Find(const FileTreeNode& rootNode, const FilterFunctor& filter, bool recurse) const
	{
		RebuildTreeIfNeeded();

		FileTreeNode::CRefVector nodes;
		FindFilesInTree(nodes, rootNode, filter, recurse);

		return nodes;
	}
	FileTreeNode::CRefVector DefaultModDispatcher::Find(const IGameMod& mod, const FilterFunctor& filter, bool recurse) const
	{
		RebuildTreeIfNeeded();

		FileTreeNode::CRefVector nodes;
		FindFilesInTree(nodes, mod.GetFileTree(), filter, recurse);

		return nodes;
	}

	KDispatcherCollision::Vector DefaultModDispatcher::FindCollisions(const IGameMod& scannedMod, const wxString& relativePath) const
	{
		KDispatcherCollision::Vector collisions;
		KMMDispatcherCollisionType type = KMM_DCT_OVERWRITTEN;
		auto CheckMod = [&scannedMod, &collisions, &type, &relativePath](const IGameMod& currentMod)
		{
			// Flip collision type after this mod is found
			if (&currentMod == &scannedMod)
			{
				type = KMM_DCT_OVERWRITES;

				// Don't count scanned mod itself
				return true;
			}

			// If this mod is enabled and it have such file - add collision info for it.
			if (currentMod.IsActive())
			{
				const FileTreeNode* node = FileTreeNode::NavigateToAny(currentMod.GetFileTree(), relativePath);
				if (node && node->IsFile())
				{
					collisions.emplace_back(currentMod, type);
				}
			}
			return true;
		};

		IterateModsBackward(CheckMod, true);
		std::reverse(collisions.begin(), collisions.end());
		return collisions;
	}

	DefaultModDispatcher::DefaultModDispatcher()
	{
		IEvent::Bind(Events::ModFilesChanged, &DefaultModDispatcher::OnVirtualTreeInvalidated, this);
		IEvent::Bind(Events::ModInstalled, &DefaultModDispatcher::OnVirtualTreeInvalidated, this);
		IEvent::Bind(Events::ModUninstalled, &DefaultModDispatcher::OnVirtualTreeInvalidated, this);
		IEvent::Bind(Events::ModsReordered, &DefaultModDispatcher::OnVirtualTreeInvalidated, this);
	}
}
