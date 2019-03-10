#pragma once
#include "stdafx.h"
#include "FileTreeNode.h"
#include "IGameMod.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IGameMod;

	class IModDispatcher: public KxSingletonPtr<IModDispatcher>
	{
		public:
			enum class IterationOrder
			{
				Direct,
				Reversed
			};
			using IterationFunctor = std::function<bool(const IGameMod&)>;
			using FilterFunctor = std::function<bool(const FileTreeNode&)>;

		protected:
			IGameMod* DoIterateMods(const IGameMod::RefVector& mods, const IterationFunctor& functor, IterationOrder order) const;

		public:
			// Full rebuild of file tree. Invalidates all references to old tree nodes.
			virtual void UpdateVirtualTree() = 0;
			virtual void InvalidateVirtualTree() = 0;

			// Root node of virtual files tree.
			virtual const FileTreeNode& GetVirtualTree() const = 0;

			// Resolves provided relative file path to real file.
			virtual const FileTreeNode* ResolveLocation(const wxString& relativePath) const = 0;

			// A different variant of 'ResolveLocation'. If 'relativePath' is not found, returns it as relative to write target.
			// Returns absolute paths unchanged.
			virtual wxString ResolveLocationPath(const wxString& relativePath, const IGameMod** owningMod = nullptr) const = 0;

			// Searches virtual tree for specified file given its fill path.
			virtual const FileTreeNode* BackTrackFullPath(const wxString& fullPath) const = 0;
		
			// Searches files in virtual tree in specified directory.
			virtual FileTreeNode::CRefVector Find(const wxString& relativePath, const FilterFunctor& filter, bool recurse = false) const = 0;

			// Searches files in specified node. This can be 'BasicGameMod' tree or virtual tree.
			virtual FileTreeNode::CRefVector Find(const FileTreeNode& rootNode, const FilterFunctor& filter, bool recurse = false) const = 0;
		
			// Searches files in specified mod. It's basically short-circuit to previous function with mod's file tree.
			virtual FileTreeNode::CRefVector Find(const IGameMod& mod, const FilterFunctor& filter, bool recurse = false) const = 0;

			// Iterates over all mods in specified order calling provided functor.
			// Returns non-null if iteration stopped before reaching the end.
			// Return 'false' from functor to stop iteration.
			IGameMod* IterateModsForward(const IGameMod::RefVector& mods, IterationFunctor functor) const
			{
				return DoIterateMods(mods, functor, IterationOrder::Direct);
			}
			IGameMod* IterateModsBackward(const IGameMod::RefVector& mods, IterationFunctor functor) const
			{
				return DoIterateMods(mods, functor, IterationOrder::Reversed);
			}
			IGameMod* IterateModsForward(IterationFunctor functor, bool includeWriteTarget = false) const;
			IGameMod* IterateModsBackward(IterationFunctor functor, bool includeWriteTarget = false) const;
	};
}
