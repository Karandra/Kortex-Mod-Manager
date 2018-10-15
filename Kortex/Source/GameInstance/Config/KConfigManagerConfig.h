#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KxXMLNode;
class KGameInstance;
enum KCMTypeDetector;

enum KPGCFormat
{
	KPGC_FORMAT_INVALID = -1,
	KPGC_FORMAT_INI,
};
enum KPGCFileID: int
{
	KPGC_ID_INVALID = INT_MIN,

	KPGC_ID_MAIN = 0,
	KPGC_ID_PREFS,
	KPGC_ID_CUSTOM,

	KPGC_ID_EDITOR_MAIN = 1000,
	KPGC_ID_EDITOR_PREFS,
	KPGC_ID_EDITOR_CUSTOM,

	KPGC_ID_ENB_LOCAL = -1000,
	KPGC_ID_ENB_SERIES = -1001,
	
	KPGC_ID_INSTANCE = INT_MAX - 1,
	KPGC_ID_APP = INT_MAX,
};

class KConfigManagerConfigEntry
{
	public:
		static KPGCFormat NameToFormatID(const wxString& format);

	private:
		KPGCFileID m_ID = KPGC_ID_INVALID;
		KPGCFormat m_Format = KPGC_FORMAT_INVALID;
		KCMTypeDetector m_TypeDetectorID;
		wxString m_FilePath;

	public:
		KConfigManagerConfigEntry(KxXMLNode& node);
		KConfigManagerConfigEntry(KPGCFileID id, KPGCFormat format, KCMTypeDetector typeDetectorID, const wxString& filePath);

	public:
		bool IsOK() const
		{
			return m_ID != KPGC_ID_INVALID;
		}
		bool IsGameConfigID() const
		{
			return IsOK() && m_ID != KPGC_ID_APP && m_ID != KPGC_ID_INSTANCE;
		}
		bool IsENBID() const
		{
			return IsOK() && (m_ID == KPGC_ID_ENB_LOCAL || m_ID == KPGC_ID_ENB_SERIES);
		}
		
		KPGCFileID GetID() const
		{
			return m_ID;
		}
		KPGCFormat GetFormat() const
		{
			return m_Format;
		}
		KCMTypeDetector GetTypeDetectorID() const
		{
			return m_TypeDetectorID;
		}
		
		const wxString& GetFilePath() const
		{
			return m_FilePath;
		}
		wxString GetFileName() const
		{
			return m_FilePath.AfterLast('\\');
		}
		
		bool IsFilePathAbsolute() const
		{
			return m_FilePath.Length() >= 2 && m_FilePath[1] == ':';
		}
		bool IsFilePathRelative() const
		{
			return !m_FilePath.IsEmpty() && !IsFilePathAbsolute();
		}
};

//////////////////////////////////////////////////////////////////////////
class KConfigManagerConfig: public KxSingletonPtr<KConfigManagerConfig>
{
	private:
		bool m_EnableENB = false;
		std::vector<KConfigManagerConfigEntry> m_Entries;

	public:
		KConfigManagerConfig(KGameInstance& profile, const KxXMLNode& node);

	public:
		bool IsENBEnabled() const
		{
			return m_EnableENB;
		}
		
		size_t GetEntriesCount() const
		{
			return m_Entries.size();
		}
		const KConfigManagerConfigEntry* GetEntryAt(size_t i) const
		{
			if (i < m_Entries.size())
			{
				return &m_Entries[i];
			}
			return NULL;
		}
		const KConfigManagerConfigEntry* GetEntry(KPGCFileID id) const;
};
