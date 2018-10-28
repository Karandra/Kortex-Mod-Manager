#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KModManager;
class KModEntry;
class KGameInstance;

class KModTag: public KLabeledValue
{
	public:
		using Vector = std::vector<KModTag>;
		using RefVector = std::vector<KModTag*>;
		using CRefVector = std::vector<const KModTag*>;

	private:
		KxColor m_Color;
		int m_NexusID = -1;
		bool m_IsSystemTag = false;

	public:
		KModTag(const wxString& value, const wxString& label = wxEmptyString, bool isSystemTag = false);
		virtual ~KModTag();

	public:
		bool HasColor() const
		{
			return m_Color.IsOk();
		}
		KxColor GetColor() const
		{
			return m_Color;
		}
		void SetColor(const KxColor& color)
		{
			m_Color = color;
		}

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

//////////////////////////////////////////////////////////////////////////
class KModTagsManager: public KxSingletonPtr<KModTagsManager>
{
	friend class KModManager;

	private:
		KModTag::Vector m_Tags;

	private:
		wxString GetUserTagsFile() const;
		void LoadTagsFromFile(const wxString& filePath);
		void LoadDefaultTags();
		
		void OnInit();
		void LoadUserTags();
		void SaveUserTags() const;

	public:
		KModTagsManager();
		virtual ~KModTagsManager();

	public:
		KModTag::Vector& GetTags()
		{
			return m_Tags;
		}
		size_t GetTagsCount() const
		{
			return m_Tags.size();
		}
		bool HasTags() const
		{
			return m_Tags.empty();
		}
		
		const KModTag* FindModTag(const wxString& tagValue, KModTag::Vector::const_iterator* iterator = NULL) const;
		KModTag* FindModTag(const wxString& tagValue, KModTag::Vector::const_iterator* iterator = NULL);
		
		KModTag* AddModTag(const KModTag& tag);
		KModTag* AddModTag(const wxString& name);

		bool RemoveModTag(const wxString& tagValue);
		const wxString& GetTagName(const wxString& tagID) const;

		void LoadTagsFromEntry(const KModEntry& entry);
};
