#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
class KModManager;
class KModEntry;
class KGameInstance;

class KModTag: public KLabeledValue
{
	private:
		int m_NexusID = -1;
		bool m_IsSystemTag = false;

	public:
		KModTag(const wxString& value, const wxString& label = wxEmptyString, bool isSystemTag = false);
		virtual ~KModTag();

	public:
		bool HasNexusID() const
		{
			return m_NexusID >= 0;
		}
		int GetNexusID() const
		{
			return m_NexusID;
		}
		void SetNexusID(int value)
		{
			m_NexusID = value;
		}

		bool IsSystemTag() const
		{
			return m_IsSystemTag;
		}
};
typedef std::vector<KModTag> KModTagArray;

//////////////////////////////////////////////////////////////////////////
class KModTagsManager
{
	friend class KModManager;

	public:
		static wxString GetDefaultTagsFile();

	private:
		KModTagArray m_Tags;

	private:
		void LoadTagsFromFile(const wxString& filePath);
		void OnInit();

	public:
		KModTagsManager();
		virtual ~KModTagsManager();

	public:
		KModTagArray& GetTagList()
		{
			return m_Tags;
		}
		size_t GetTagsCount() const
		{
			return m_Tags.size();
		}
		bool IsTagListEmpty() const
		{
			return m_Tags.empty();
		}
		
		const KModTag* FindModTag(const wxString& tagValue, KModTagArray::const_iterator* pIt = NULL) const;
		const KModTag* AddModTag(const KModTag& tag);
		const KModTag* AddModTag(const wxString& name)
		{
			// Add tag with value only.
			return AddModTag(KModTag(name));
		}
		bool RemoveModTag(const wxString& tagValue);
		const wxString& GetTagName(const wxString& tagID) const;
		void LoadTagsFromEntry(const KModEntry* entry);

		wxString GetUserTagsFile() const;
		void LoadDefaultTags()
		{
			LoadTagsFromFile(GetDefaultTagsFile());
		}
		void LoadUserTags()
		{
			LoadTagsFromFile(GetUserTagsFile());
		}
		void SaveUserTags() const;
};
