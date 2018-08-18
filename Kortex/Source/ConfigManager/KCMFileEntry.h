#pragma once
#include "stdafx.h"
#include "KCMConfigEntry.h"
class KConfigManager;
class KConfigManagerConfigEntry;
class KCMIDataProvider;
enum KPGCFileID;

class KCMFileEntry
{
	private:
		KConfigManager* m_ConfigManager = NULL;
		const KConfigManagerConfigEntry* m_ProfileEntry = NULL;
		KCMOptionsFormatter m_Formatter;
		KCMConfigEntriesArray m_Entries;
		KCMIDataProvider* m_DataProvider = NULL;

	private:
		KPGCFileID LoadID(KxXMLNode& node);
		void LoadEntries(KxXMLNode& node);
		void LoadUnknownEntries();
		void InitDataProvider();

	public:
		KCMFileEntry(KConfigManager* pConfigManager, KxXMLNode& node, const KCMOptionsFormatter& tDefaultOptions);
		KCMFileEntry(KConfigManager* pConfigManager, const KConfigManagerConfigEntry* pProfileEntry, const KCMOptionsFormatter& tDefaultOptions);
		void ClearEntries();
		virtual ~KCMFileEntry();

	public:
		bool IsOK() const;
		KPGCFileID GetID() const;
		bool IsPartOfENB() const;
		KConfigManager* GetConfigManager() const
		{
			return m_ConfigManager;
		}
		const KConfigManagerConfigEntry* GetProfileEntry() const
		{
			return m_ProfileEntry;
		}
		KCMIDataProvider* GetProvider() const
		{
			return m_DataProvider;
		}
		const KCMOptionsFormatter& GetFormatter() const
		{
			return m_Formatter;
		}

		size_t GetEntriesCount() const
		{
			return m_Entries.size();
		}
		KCMConfigEntryBase* GetEntryAt(size_t i) const
		{
			if (i < m_Entries.size())
			{
				return m_Entries[i];
			}
			return NULL;
		}
		void AddEntry(KCMConfigEntryBase* entry)
		{
			return m_Entries.push_back(entry);
		}

		void Load();
};
typedef std::vector<KCMFileEntry*> KCMFileEntryArray;
