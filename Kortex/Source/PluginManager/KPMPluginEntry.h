#pragma once
#include "stdafx.h"
class KModEntry;
class KPMPluginReader;
class KPluginManagerConfigStandardContentEntry;

enum KPMPluginEntryType
{
	KPMPE_TYPE_INVALID = 0,
	KPMPE_TYPE_ALL = -1,

	KPMPE_TYPE_NORMAL = 1 << 0,
	KPMPE_TYPE_MASTER = 1 << 1,
	KPMPE_TYPE_LIGHT = 1 << 2,
};

KPMPluginEntryType operator|(const KPMPluginEntryType& arg1, const KPMPluginEntryType& arg2);
KPMPluginEntryType& operator|=(KPMPluginEntryType& arg1, const KPMPluginEntryType& arg2);

class KPMPluginEntry
{
	private:
		wxString m_Name;
		wxString m_FullPath;

		KPMPluginEntryType m_Type = KPMPE_TYPE_INVALID;
		bool m_IsEnabled = false;
		const KModEntry* m_ParentMod = NULL;
		mutable KPMPluginReader* m_PluginReader = NULL;

		mutable bool m_StdContentEntryChecked = false;
		mutable const KPluginManagerConfigStandardContentEntry* m_StdContentEntry = NULL;

	public:
		KPMPluginEntry(const wxString& name, bool isActive, KPMPluginEntryType type);
		virtual ~KPMPluginEntry();

	public:
		bool IsOK() const
		{
			return !m_Name.IsEmpty();
		}
		virtual void OnUpdate()
		{
		}

		const wxString& GetName() const
		{
			return m_Name;
		}
		void SetName(const wxString& name)
		{
			m_Name = name;
		}
		
		const wxString& GetFullPath() const
		{
			return m_FullPath;
		}
		void SetFullPath(const wxString& fullPath)
		{
			m_FullPath = fullPath;
		}

		virtual bool CanToggleEnabled() const;
		virtual bool IsEnabled() const;
		virtual void SetEnabled(bool isActive);
		
		virtual KPMPluginEntryType GetFormat() const;
		void SetFormat(KPMPluginEntryType type);

		virtual const KModEntry* GetParentMod() const
		{
			return m_ParentMod;
		}
		void SetParentMod(const KModEntry* modEntry)
		{
			m_ParentMod = modEntry;
		}

		virtual KPMPluginReader* GetPluginReader() const;
		const KPluginManagerConfigStandardContentEntry* GetStdContentEntry() const;
};
typedef std::vector<std::unique_ptr<KPMPluginEntry>> KPMPluginEntryVector;
typedef std::vector<KPMPluginEntry*> KPMPluginEntryRefVector;
typedef std::vector<const KPMPluginEntry*> KPMPluginEntryCRefVector;
