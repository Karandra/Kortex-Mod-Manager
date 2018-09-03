#pragma once
#include "stdafx.h"
#include "KFileTreeNode.h"
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
				return KxString::ToLower(lhs) == KxString::ToLower(rhs);
			}
		};
		using FinderHash = std::unordered_set<wxString, std::hash<wxString>, FinderHashComparator>;

	private:
		void FindFilesInTree(KFileTreeNode::CRefVector& nodes,
							 FinderHash* hash,
							 const KFileTreeNode& rootNode,
							 const wxString& filter,
							 KxFileSearchType type,
							 bool recurse,
							 const wxString& absolutePath = wxEmptyString
		) const;

	public:
		KModEntry* IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget = true, bool activeOnly = true) const;
		
		wxString GetTargetPath(const wxString& relativePath, KModEntry** owningMod = NULL) const;
		
		KFileTreeNode::CRefVector FindFiles(const KFileTreeNode& rootNode, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, FinderHash* hash = NULL) const;
		KFileTreeNode::CRefVector FindFiles(const KModEntry& modEntry, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, FinderHash* hash = NULL) const;
		KFileTreeNode::CRefVector FindFiles(const wxString& relativePath, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false, FinderHash* hash = NULL) const;
		
		CollisionVector FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const;
};
