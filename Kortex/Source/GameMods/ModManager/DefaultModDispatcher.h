#pragma once
#include "stdafx.h"
#include "GameMods/IModDispatcher.h"
#include "Application/BroadcastProcessor.h"
#include <KxFramework/KxFile.h>

namespace Kortex::ModManager
{
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
			const IGameMod* m_Mod = nullptr;
			KMMDispatcherCollisionType m_Type = KMM_DCT_NONE;

		public:
			KDispatcherCollision(const IGameMod& mod, KMMDispatcherCollisionType type)
				:m_Mod(&mod), m_Type(type)
			{
			}

		public:
			const IGameMod& GetMod() const
			{
				return *m_Mod;
			}
			KMMDispatcherCollisionType GetType() const
			{
				return m_Type;
			}
	};
}

namespace Kortex::ModManager
{
	class DispatcherSearcher
	{
		private:
			const wxString m_Filter;
			const KxFileSearchType m_ElementType = KxFS_ALL;

		public:
			DispatcherSearcher(const wxString& filter = wxEmptyString, KxFileSearchType type = KxFS_FILE)
				:m_Filter(filter), m_ElementType(type)
			{
			}

		public:
			bool operator()(const FileTreeNode& node) const;
	};
}

namespace Kortex::ModManager
{
	class DefaultModDispatcher: public IModDispatcher
	{
		private:
			BroadcastReciever m_BroadcastReciever;
			FileTreeNode m_VirtualTree;

		private:
			void OnVirtualTreeInvalidated(BroadcastEvent& event);

		public:
			void InvalidateVirtualTree() override;
			void UpdateVirtualTree() override;

			const FileTreeNode& GetVirtualTree() const override;

			const FileTreeNode* ResolveLocation(const wxString& relativePath) const override;
			wxString ResolveLocationPath(const wxString& relativePath, const IGameMod** owningMod = nullptr) const override;
			const FileTreeNode* BackTrackFullPath(const wxString& fullPath) const override;

			FileTreeNode::CRefVector Find(const wxString& relativePath, const FilterFunctor& filter, bool recurse = false) const override;
			FileTreeNode::CRefVector Find(const FileTreeNode& rootNode, const FilterFunctor& filter, bool recurse = false) const override;
			FileTreeNode::CRefVector Find(const IGameMod& mod, const FilterFunctor& filter, bool recurse = false) const override;

			// Searches for collisions of file specified by 'relativePath' for a mod.
			KDispatcherCollision::Vector FindCollisions(const IGameMod& scannedMod, const wxString& relativePath) const;

		public:
			DefaultModDispatcher();
	};
}
