#pragma once
#include "stdafx.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
class KModEntry;
class KFileTreeNode;

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

	private:
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
		KModEntry* IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget = true) const;

		void FindFilesInTree(FilesVector& files,
							 FinderHash* hash,
							 const KFileTreeNode& rootNode,
							 const wxString& filter,
							 KxFileSearchType type,
							 bool recurse 
		) const;

	public:
		wxString GetTargetPath(const wxString& relativePath, KModEntry** owningMod = NULL) const;
		
		FilesVector FindFiles(const KModEntry& modEntry, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false) const;
		FilesVector FindFiles(const wxString& relativePath, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false) const;
		
		CollisionVector FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const;
};
