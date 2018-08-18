#pragma once
#include "stdafx.h"
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
		KModEntry* m_Mod = NULL;
		KMMDispatcherCollisionType m_Type = KMM_DCT_NONE;

	public:
		KMMDispatcherCollision(KModEntry* mod, KMMDispatcherCollisionType type)
			:m_Mod(mod), m_Type(type)
		{
		}

	public:
		KModEntry* GetMod() const
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

	private:
		template<class Functor> KModEntry* IterateOverMods(Functor& functor, IterationOrder order) const
		{
			KModEntryArray entries = KModManager::Get().GetAllEntries(true);
			if (order == IterationOrder::Direct)
			{
				for (KModEntry* entry: entries)
				{
					if (functor(entry))
					{
						return entry;
					}
				}
			}
			else
			{
				for (auto it = entries.rbegin(); it != entries.rend(); ++it)
				{
					if (functor(*it))
					{
						return *it;
					}
				}
			}
			return NULL;
		}

	public:
		wxString GetTargetPath(const wxString& relativePath, KModEntry** owningMod = NULL) const;
		FilesVector FindFiles(const wxString& relativePath, const wxString& filter = KxFile::NullFilter, KxFileSearchType type = KxFS_ALL, bool recurse = false) const;
		CollisionVector FindCollisions(const KModEntry* scannedMod, const wxString& relativePath) const;
};
