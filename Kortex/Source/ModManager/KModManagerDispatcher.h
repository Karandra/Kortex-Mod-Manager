#pragma once
#include "stdafx.h"
#include "KFileTreeNode.h"
#include "KEvents.h"
#include "KComparator.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
class KModEntry;

enum KMMDispatcherCollisionType
{
	KMM_DCT_NONE,
	KMM_DCT_UNKNOWN,
	KMM_DCT_OVERWRITTEN,
	KMM_DCT_OVERWRITES,
};

class KMMDispatcherCollision
{
	public:
		static wxString GetLocalizedCollisionName(KMMDispatcherCollisionType type);

	private:
		const KModEntry* m_Mod = NULL;
		KMMDispatcherCollisionType m_Type = KMM_DCT_NONE;

	public:
		KMMDispatcherCollision(const KModEntry* mod, KMMDispatcherCollisionType type)
			:m_Mod(mod), m_Type(type)
		{
		}

	public:
		const KModEntry* GetMod() const
		{
			return m_Mod;
		}
		KMMDispatcherCollisionType GetType() const
		{
			return m_Type;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModManagerDispatcher
{
	public:
		using CollisionVector = std::vector<KMMDispatcherCollision>;
		using FilesVector = std::vector<KxFileFinderItem>;
		using ModsVector = std::vector<KModEntry*>;

		enum class IterationOrder
		{
			Direct,
			Reversed
		};
		using IterationFunctor = std::function<bool(const KModEntry&)>;

		struct FinderHashComparator
		{
			bool operator()(const wxString& lhs, const wxString& rhs) const
			{
				return KComparator::KEqual(lhs, rhs, true);
			}
		};
		using FinderHash = std::unordered_set<wxString, std::hash<wxString>, FinderHashComparator>;

	private:
		KFileTreeNode m_VirtualTree;
		bool m_VirtualTreeNeedsRefresh = false;

	private:
		KModEntry* IterateOverModsEx(const ModsVector& mods, const IterationFunctor& functor, IterationOrder order, bool activeOnly, bool realMode) const;
		bool CheckConditionsAndCallFunctor(const IterationFunctor& functor, const KModEntry& modEntry, bool activeOnly, bool realMode) const;

		void BuildTreeBranch(const ModsVector& mods, KFileTreeNode::Vector& children, const KFileTreeNode* rootNode, KFileTreeNode::RefVector& directories);
		void RebuildTreeIfNeeded() const;

		void OnVirtualTreeInvalidated(KModEvent& event);

	public:
		// Root node to virtual files tree.
		const KFileTreeNode& GetVirtualTree() const
		{
			return m_VirtualTree;
		}
		
		// Full rebuild of file tree. Invalidates all references to old tree nodes.
		void UpdateVirtualTree();

		// Iterates over all mods in specified order calling provided functor.
		KModEntry* IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget = true, bool activeOnly = true) const;
		
		// Resolves provided relative file path to real file.
		const KFileTreeNode* ResolveLocation(const wxString& relativePath) const;

		// A different variant of 'ResolveLocation'. If 'relativePath' is not found, returns it as relative to write target.
		// Returns absolute paths unchanged.
		wxString ResolveLocationPath(const wxString& relativePath, const KModEntry** owningMod = NULL) const;
		
		// Searches files in specified node. This can be 'KModEntry' tree or virtual tree.
		KFileTreeNode::CRefVector FindFiles(const KFileTreeNode& rootNode, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, FinderHash* hash = NULL) const;
		
		// Searches files in specified mod. It's basically short-circuit to previous function with mod's file tree.
		KFileTreeNode::CRefVector FindFiles(const KModEntry& modEntry, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, FinderHash* hash = NULL) const;
		
		// Searches files in virtual tree in specified directory.
		KFileTreeNode::CRefVector FindFiles(const wxString& relativePath, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, FinderHash* hash = NULL) const;
		
		// Searches for collisions of file specified by 'relativePath' for a mod.
		CollisionVector FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const;

	public:
		KModManagerDispatcher();
};
