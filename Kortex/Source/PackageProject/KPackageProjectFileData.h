#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
class KPPFFolderEntry;

class KPPFFileEntry
{
	private:
		wxString m_ID;
		wxString m_Source;
		wxString m_Destination;
		int32_t m_Priority;

	public:
		KPPFFileEntry();
		virtual ~KPPFFileEntry();

	public:
		virtual KPPFFolderEntry* ToFolderEntry()
		{
			return NULL;
		}
		virtual const KPPFFolderEntry* ToFolderEntry() const
		{
			return NULL;
		}
		virtual KPPFFileEntry* ToFileEntry()
		{
			return this;
		}
		virtual const KPPFFileEntry* ToFileEntry() const
		{
			return this;
		}

	public:
		const wxString& GetID() const
		{
			return m_ID.IsEmpty() ? m_Source : m_ID;
		}
		void SetID(const wxString& id)
		{
			m_ID = id;
		}
		void MakeUniqueID();

		const wxString& GetSource() const
		{
			return m_Source;
		}
		void SetSource(const wxString& value)
		{
			m_Source = value;
		}
		
		const wxString& GetDestination() const
		{
			return m_Destination;
		}
		void SetDestination(const wxString& value)
		{
			m_Destination = value;
		}

		bool IsDefaultPriority() const;
		int32_t GetPriority() const;
		void SetPriority(int32_t value);
};
typedef std::vector<std::unique_ptr<KPPFFileEntry>> KPPFFileEntryArray;
typedef std::vector<KPPFFileEntry*> KPPFFileEntryRefArray;

//////////////////////////////////////////////////////////////////////////
class KPPFFolderEntryItem
{
	private:
		wxString m_Source;
		wxString m_Target;

	public:
		KPPFFolderEntryItem()
		{
		}
		~KPPFFolderEntryItem()
		{
		}

	public:
		const wxString& GetSource() const
		{
			return m_Source;
		}
		void SetSource(const wxString& value)
		{
			m_Source = value;
		}
		
		const wxString& GetDestination() const
		{
			return m_Target;
		}
		void SetDestination(const wxString& value)
		{
			m_Target = value;
		}
};
typedef std::vector<KPPFFolderEntryItem> KPPFFolderItemsArray;

//////////////////////////////////////////////////////////////////////////
class KPPFFolderEntry: public KPPFFileEntry
{
	private:
		KPPFFolderItemsArray m_Files;

	public:
		KPPFFolderEntry();
		virtual ~KPPFFolderEntry();

	public:
		virtual KPPFFolderEntry* ToFolderEntry()
		{
			return this;
		}
		virtual const KPPFFolderEntry* ToFolderEntry() const
		{
			return this;
		}

	public:
		KPPFFolderItemsArray& GetFiles()
		{
			return m_Files;
		}
		const KPPFFolderItemsArray& GetFiles() const
		{
			return m_Files;
		}
};

//////////////////////////////////////////////////////////////////////////
class KPackageProjectFileData: public KPackageProjectPart
{
	public:
		static const int ms_DefaultPriority = -1;
		static const int ms_MinUserPriority = -1;
		static const int ms_MaxUserPriority = UINT16_MAX;

	public:
		static bool IsPriorityValid(int32_t value)
		{
			return value >= KPackageProjectFileData::ms_MinUserPriority && value <= KPackageProjectFileData::ms_MaxUserPriority;
		}
		static bool IsFileIDValid(const wxString& id);

	private:
		KPPFFileEntryArray m_Data;

	public:
		KPackageProjectFileData(KPackageProject& project);
		virtual ~KPackageProjectFileData();

	public:
		KPPFFileEntryArray& GetData()
		{
			return m_Data;
		}
		const KPPFFileEntryArray& GetData() const
		{
			return m_Data;
		}

		KPPFFileEntry* AddFile(KPPFFileEntry* entry)
		{
			m_Data.push_back(std::unique_ptr<KPPFFileEntry>(entry));
			return entry;
		}
		KPPFFolderEntry* AddFolder(KPPFFolderEntry* entry)
		{
			m_Data.push_back(std::unique_ptr<KPPFFolderEntry>(entry));
			return entry;
		}

		KPPFFileEntry* FindEntryWithID(const wxString& id, size_t* index = NULL) const;
		bool HasEntryWithID(const wxString& id, const KPPFFileEntry* ignoreThis = NULL) const
		{
			KPPFFileEntry* entry = FindEntryWithID(id);
			return entry != NULL && entry != ignoreThis;
		}
		bool CanUseThisIDForNewEntry(const wxString& id) const
		{
			return IsFileIDValid(id) && !HasEntryWithID(id);
		}
};
