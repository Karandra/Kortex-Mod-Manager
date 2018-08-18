#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
class KModEntry;
class KProfile;

class KModTag: public KLabeledValue
{
	private:
		bool m_IsSystemTag = false;

	public:
		KModTag(const wxString& value, const wxString& label = wxEmptyString, bool bSystemTag = false);
		~KModTag();

	public:
		bool IsSystemTag() const
		{
			return m_IsSystemTag;
		}
};
typedef std::vector<KModTag> KModTagArray;

//////////////////////////////////////////////////////////////////////////
class KModManagerTags
{
	public:
		static wxString GetDefaultTagsFile();
		static wxString GetUserTagsFile(const wxString& templateID, const wxString& configID);

	private:
		KModTagArray m_Tags;

	private:
		void LoadTagsFromFile(const wxString& filePath);

	public:
		KModManagerTags();
		virtual ~KModManagerTags();

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
