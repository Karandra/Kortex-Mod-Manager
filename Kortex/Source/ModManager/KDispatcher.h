#pragma once
#include "stdafx.h"
#include "KFileTreeNode.h"
#include "KEventsFwd.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxSingleton.h>
class KModEntry;

enum KMMDispatcherCollisionType
{
	KMM_DCT_NONE,
	KMM_DCT_UNKNOWN,
	KMM_DCT_OVERWRITTEN,
	KMM_DCT_OVERWRITES,
};

class KDispatcherCollision
{
	public:
		using Vector = std::vector<KDispatcherCollision>;

	public:
		static wxString GetLocalizedCollisionName(KMMDispatcherCollisionType type);

	private:
		const KModEntry* m_Mod = NULL;
		KMMDispatcherCollisionType m_Type = KMM_DCT_NONE;

	public:
		KDispatcherCollision(const KModEntry* mod, KMMDispatcherCollisionType type)
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
class KDispatcher: public KxSingletonPtr<KDispatcher>
{
	public:
		using FilesVector = std::vector<KxFileItem>;
		using ModsVector = std::vector<KModEntry*>;

		enum class IterationOrder
		{
			Direct,
			Reversed
		};
		using IterationFunctor = std::function<bool(const KModEntry&)>;

	private:
		KFileTreeNode m_VirtualTree;
		bool m_VirtualTreeInvalidated = false;

	private:
		KModEntry* IterateOverModsEx(const ModsVector& mods, const IterationFunctor& functor, IterationOrder order, bool activeOnly) const;
		bool CheckConditionsAndCallFunctor(const IterationFunctor& functor, const KModEntry& modEntry, bool activeOnly) const;

		void RebuildTreeIfNeeded() const;
		void OnVirtualTreeInvalidated(KEvent& event);

	public:
		// Full rebuild of file tree. Invalidates all references to old tree nodes.
		void UpdateVirtualTree();
		void InvalidateVirtualTree();

		// Root node of virtual files tree.
		const KFileTreeNode& GetVirtualTree() const;

		// Iterates over all mods in specified order calling provided functor.
		KModEntry* IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget = true, bool activeOnly = true) const;
		
		// Resolves provided relative file path to real file.
		const KFileTreeNode* ResolveLocation(const wxString& relativePath) const;

		// A different variant of 'ResolveLocation'. If 'relativePath' is not found, returns it as relative to write target.
		// Returns absolute paths unchanged.
		wxString ResolveLocationPath(const wxString& relativePath, const KModEntry** owningMod = NULL) const;

		// Searches virtual tree for specified file given its fill path.
		const KFileTreeNode* BackTrackFullPath(const wxString& fullPath) const;
		
		// Searches files in virtual tree in specified directory.
		KFileTreeNode::CRefVector FindFiles(const wxString& relativePath, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, bool activeOnly = false) const;

		// Searches files in specified node. This can be 'KModEntry' tree or virtual tree.
		KFileTreeNode::CRefVector FindFiles(const KFileTreeNode& rootNode, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, bool activeOnly = false) const;
		
		// Searches files in specified mod. It's basically short-circuit to previous function with mod's file tree.
		KFileTreeNode::CRefVector FindFiles(const KModEntry& modEntry, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false) const;
		
		// Searches for collisions of file specified by 'relativePath' for a mod.
		KDispatcherCollision::Vector FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const;

	public:
		KDispatcher();
};
