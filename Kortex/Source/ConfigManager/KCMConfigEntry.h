#pragma once
#include "stdafx.h"
#include <KxFramework/KxTreeList.h>
#include "Utility/KEntryRefT.h"
#include "KCMOptionsFormatter.h"
#include "KCMSampleValue.h"
enum KPGCFileID;
class KCMFileEntry;
class KxINI;
class KxXMLDocument;

class KCMConfigEntryPath;
class KCMConfigEntryStd;
class KCMConfigEntryDV;
class KCMConfigEntryVK;
class KCMConfigEntryArray;
class KCMConfigEntryFileBrowse;

enum KCMDataType
{
	KCMDT_UNKNOWN = -1,

	KCMDT_INT8,
	KCMDT_UINT8,
	KCMDT_INT16,
	KCMDT_UINT16,
	KCMDT_INT32,
	KCMDT_UINT32,
	KCMDT_INT64,
	KCMDT_UINT64,
	KCMDT_FLOAT32,
	KCMDT_FLOAT64,
	KCMDT_BOOL,
	KCMDT_STRING,

	KCMDT_MAX,
};
enum KCMDataSubType
{
	KCMDST_NONE,
	KCMDST_ARRAY,
	KCMDST_DOUBLE_VALUE,
	KCMDST_VIRTUAL_KEY,
	KCMDST_FILE_BROWSE,
};
enum KCMEditableBehavior
{
	KCMEB_NON_EDITABLE = 0,
	KCMEB_EDITABLE = 1,
	KCMEB_CONTEXT = -1,
};

class KCMConfigEntryBase
{
	protected:
		KCMFileEntry* m_FileEntry = NULL;
		KxTreeListItem m_ViewNode;
		bool m_IsUnknownEntry = false;

	public:
		KCMConfigEntryBase(KCMFileEntry* fileEntry);
		virtual ~KCMConfigEntryBase();

	public:
		virtual bool IsOK() const
		{
			return m_FileEntry != NULL;
		}
		KCMFileEntry* GetFileEntry() const
		{
			return m_FileEntry;
		}
		KxTreeListItem GetViewNode() const
		{
			return m_ViewNode;
		}
		void SetViewNode(const KxTreeListItem& node)
		{
			m_ViewNode = node;
		}
		bool IsUnknownEntry() const
		{
			return m_IsUnknownEntry;
		}
		void SetUnknownEntry(bool isUnknown)
		{
			m_IsUnknownEntry = isUnknown;
		}

	public:
		virtual KCMConfigEntryBase* ToBaseEntry()
		{
			return this;
		}
		virtual KCMConfigEntryPath* ToPathEntry()
		{
			return NULL;
		}
		virtual KCMConfigEntryStd* ToStdEntry()
		{
			return NULL;
		}
		virtual KCMConfigEntryDV* ToDVEntry()
		{
			return NULL;
		}
		virtual KCMConfigEntryVK* ToVKEntry()
		{
			return NULL;
		}
		virtual KCMConfigEntryArray* ToArrayEntry()
		{
			return NULL;
		}
		virtual KCMConfigEntryFileBrowse* ToFileBrowseEntry()
		{
			return NULL;
		}
};
typedef std::vector<KCMConfigEntryBase*> KCMConfigEntriesArray;

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryPath: public KCMConfigEntryBase
{
	protected:
		wxString m_Path;

	public:
		KCMConfigEntryPath(KCMFileEntry* fileEntry, const wxString& path);
		virtual ~KCMConfigEntryPath();

	public:
		virtual void RemovePath(KxINI& tDocument);
		virtual void RemovePath(KxXMLDocument& tDocument);

	public:
		virtual wxString GetPath() const
		{
			return m_Path;
		}
		wxString GetFullPathFor(const wxString& name) const;
		virtual wxString GetFullPath() const
		{
			return GetFullPathFor(wxEmptyString);
		}

	public:
		virtual KCMConfigEntryPath* ToPathEntry() override
		{
			return this;
		}
};

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryStd: public KCMConfigEntryPath
{
	public:
		typedef std::pair<wxString, bool> DataType;
		typedef std::pair<std::variant<int64_t, uint64_t, double>, bool> MinMaxType;

		static KCMDataSubType GetSubType(KxXMLNode& node);
		wxString FormatToDisplay(const DataType& data, KCMDataType type = KCMDT_UNKNOWN) const;

	private:
		DataType m_Data;

	protected:
		wxString m_Category;
		wxString m_Name;
		wxString m_Label;
		KCMDataType m_Type;
		KCMOptionsFormatter m_Formatter;

		KCMSampleValueArray m_SampleValues;
		MinMaxType m_MinValue;
		MinMaxType m_MaxValue;
		bool m_SortSampleValues = false;
		KCMEditableBehavior m_EditableBehavior = KCMEB_CONTEXT;

	protected:
		KCMSampleValueArray& GetSampleValuesEditable()
		{
			return m_SampleValues;
		}

		void LoadMain(KxXMLNode& node);
		void LoadSamples(KxXMLNode& node);
		void LoadEditableBehavior(KxXMLNode& node);
		void LoadSamplesFromArray(KxXMLNode& node);
		virtual KCMSampleValue OnLoadSampleValue(KxXMLNode& node) const;
		void CreateSamplesFromSequence(KxXMLNode& node);
		void CleanSamplesArray();

	public:
		KCMConfigEntryStd(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions);
		virtual void Create(KxXMLNode& node);
		void Create(const wxString& path, const wxString& name, KCMDataType type = KCMDT_UNKNOWN);
		virtual ~KCMConfigEntryStd();

	public:
		virtual void SaveEntry(KxINI& tDocument) const;
		virtual void SaveEntry(KxXMLDocument& tDocument) const;
		virtual void LoadEntry(const KxINI& tDocument);
		virtual void LoadEntry(const KxXMLDocument& tDocument);
		virtual void RemoveEntry(KxINI& tDocument);
		virtual void RemoveEntry(KxXMLDocument& tDocument);

	public:
		const KCMOptionsFormatter& GetFormatter() const
		{
			return m_Formatter;
		}
		bool HasSampleValues() const
		{
			return !m_SampleValues.empty();
		}
		const KCMSampleValueArray& GetSampleValues() const
		{
			return m_SampleValues;
		}
		const MinMaxType& GetMinValue() const
		{
			return m_MinValue;
		}
		const MinMaxType& GetMaxValue() const
		{
			return m_MaxValue;
		}
		virtual bool IsEditable() const;
		KCMDataType GetType() const
		{
			return m_Type;
		}
		wxString GetTypeName() const;
		wxString GetDisplayTypeName() const;
		const wxString& GetCategory() const
		{
			return m_Category;
		}
		const wxString& GetName() const
		{
			return m_Name;
		}
		const wxString& GetLabel() const
		{
			return m_Label;
		}
		virtual wxString GetFullPath() const override;
		virtual wxString GetDisplayData() const;
		virtual wxString OnDisplaySampleValue(const KCMSampleValue& value) const;
		virtual int FindDataInSamples(const wxString& sSearchFor = wxEmptyString) const;
		virtual bool HasData() const
		{
			return m_Data.second;
		}
		virtual wxString GetData(bool bFormat = true) const;
		virtual void SetData();
		virtual void SetData(const wxString& sData, bool bFormat = true);

		bool GetDataBool() const;
		void SetDataBool(bool value);

	public:
		virtual KCMConfigEntryStd* ToStdEntry() override
		{
			return this;
		}
};

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryDV: public KCMConfigEntryStd
{
	private:
		wxString m_Separator;
		wxString m_ValueName1;
		wxString m_ValueName2;
		DataType m_Data1;
		DataType m_Data2;

	private:
		virtual KCMSampleValue OnLoadSampleValue(KxXMLNode& node) const override;

	public:
		KCMConfigEntryDV(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions);
		virtual void Create(KxXMLNode& node) override;
		virtual ~KCMConfigEntryDV();

	public:
		virtual void SaveEntry(KxINI& tDocument) const override;
		virtual void LoadEntry(const KxINI& tDocument) override;

	public:
		const wxString& GetSeparator() const
		{
			return m_Separator;
		}
		const wxString& GetName1() const
		{
			return m_ValueName1;
		}
		const wxString& GetName2() const
		{
			return m_ValueName2;
		}
		const DataType& GetData1() const
		{
			return m_Data1;
		}
		const DataType& GetData2() const
		{
			return m_Data2;
		}
		virtual wxString GetDisplayData() const override;
		virtual wxString OnDisplaySampleValue(const KCMSampleValue& value) const override;

		virtual bool HasData() const override;
		virtual wxString GetData(bool bFormat = true) const override;
		virtual void SetData() override;
		virtual void SetData(const wxString& sData, bool bFormat = true) override;

	public:
		virtual KCMConfigEntryDV* ToDVEntry() override
		{
			return this;
		}
};

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryVK: public KCMConfigEntryStd
{
	private:
		typedef std::pair<wxKeyCode, bool> DataTypeVK;

	private:
		DataTypeVK m_DataVK;

	public:
		KCMConfigEntryVK(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions);
		virtual ~KCMConfigEntryVK();

	public:
		virtual void LoadEntry(const KxINI& tDocument) override;

	public:
		virtual bool IsEditable() const override
		{
			return true;
		}
		virtual wxString GetDisplayData() const override;
		virtual wxString OnDisplaySampleValue(const KCMSampleValue& value) const override;

		wxKeyCode GetDataKeyCode() const
		{
			return m_DataVK.first;
		}
		void SetDataKeyCode(wxKeyCode nKeyCode)
		{
			m_DataVK.first = nKeyCode;
			m_DataVK.second = true;
		}
		
		virtual bool HasData() const override
		{
			return m_DataVK.second;
		}
		virtual wxString GetData(bool bFormat = true) const override;
		virtual void SetData() override
		{
			m_DataVK.first = WXK_NONE;
			m_DataVK.second = false;
		}
		virtual void SetData(const wxString& sData, bool bFormat = true) override;

	public:
		virtual KCMConfigEntryVK* ToVKEntry() override
		{
			return this;
		}
};

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryArray: public KCMConfigEntryStd
{
	private:
		KxStringVector m_Values;
		wxString m_Separator;

	private:
		virtual void SaveEntry(KxINI& tDocument) const override;
		virtual void LoadEntry(const KxINI& tDocument) override;

	public:
		KCMConfigEntryArray(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions);
		virtual void Create(KxXMLNode& node) override;
		virtual ~KCMConfigEntryArray();

	public:
		const wxString& GetSeparator() const
		{
			return m_Separator;
		}
		const KxStringVector& GetDataArray() const
		{
			return m_Values;
		}
		KxStringVector& GetDataArray()
		{
			return m_Values;
		}
		void SetDataArray(const KxStringVector& array)
		{
			m_Values = array;
		}

		virtual wxString GetDisplayData() const override;
		wxString OnDisplaySampleValue(const KCMSampleValue& value) const;
		virtual bool HasData() const override
		{
			return !m_Values.empty();
		}
		virtual wxString GetData(bool bFormat = true) const override;
		virtual void SetData() override;
		virtual void SetData(const wxString& sData, bool bFormat = true) override;

	public:
		virtual KCMConfigEntryArray* ToArrayEntry()
		{
			return this;
		}
};

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryFileBrowse: public KCMConfigEntryStd
{
	private:
		bool m_IsFolder = false;

	public:
		KCMConfigEntryFileBrowse(KCMFileEntry* fileEntry, const KCMOptionsFormatter& tDefaultOptions);
		virtual void Create(KxXMLNode& node) override;
		virtual ~KCMConfigEntryFileBrowse();

	public:
		virtual KCMConfigEntryFileBrowse* ToFileBrowseEntry() override
		{
			return this;
		}

		bool IsFolder() const
		{
			return m_IsFolder;
		}
};

//////////////////////////////////////////////////////////////////////////
class KCMConfigEntryRef: public KEntryRef<KCMConfigEntryBase>
{
	public:
		KCMConfigEntryRef(KCMConfigEntryBase* entry)
		{
			m_Entry = entry;
		}

	public:
		void SetDeleted()
		{
			m_Entry = NULL;
		}
};
