#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "KLabeledValue.h"
#include "ModManager/KModEntry.h"

class KPackageProjectInfo: public KPackageProjectPart
{
	using FixedWebSitesArray = KModEntry::FixedWebSitesArray;

	private:
		wxString m_Name;
		wxString m_TranslatedName;
		wxString m_Version;
		wxString m_Author;
		wxString m_Translator;
		wxString m_Description;
		KLabeledValueArray m_CustomFields;
		KLabeledValueArray m_Documents;
		KLabeledValueArray m_WebSites;
		FixedWebSitesArray m_FixedWebSites = {-1, -1, -1};
		KxStringVector m_Tags;

	public:
		KPackageProjectInfo(KPackageProject& project);
		virtual ~KPackageProjectInfo();

	public:
		const wxString& GetName() const
		{
			return m_Name;
		}
		void SetName(const wxString& value)
		{
			m_Name = value;
		}
		
		const wxString& GetTranslatedName() const
		{
			return m_TranslatedName;
		}
		void SetTranslatedName(const wxString& value)
		{
			m_TranslatedName = value;
		}
		
		const wxString& GetVersion() const
		{
			return m_Version;
		}
		void SetVersion(const wxString& value)
		{
			m_Version = value;
		}
		
		const wxString& GetAuthor() const
		{
			return m_Author;
		}
		void SetAuthor(const wxString& value)
		{
			m_Author = value;
		}
		
		const wxString& GetTranslator() const
		{
			return m_Translator;
		}
		void SetTranslator(const wxString& value)
		{
			m_Translator = value;
		}
		
		const wxString& GetDescription() const
		{
			return m_Description;
		}
		void SetDescription(const wxString& value)
		{
			m_Description = value;
		}
		
		const KLabeledValueArray& GetCustomFields() const
		{
			return m_CustomFields;
		}
		KLabeledValueArray& GetCustomFields()
		{
			return m_CustomFields;
		}
		
		const KLabeledValueArray& GetDocuments() const
		{
			return m_Documents;
		}
		KLabeledValueArray& GetDocuments()
		{
			return m_Documents;
		}

		const KLabeledValueArray& GetWebSites() const
		{
			return m_WebSites;
		}
		KLabeledValueArray& GetWebSites()
		{
			return m_WebSites;
		}
		const FixedWebSitesArray& GetFixedWebSites() const
		{
			return m_FixedWebSites;
		}
		FixedWebSitesArray& GetFixedWebSites()
		{
			return m_FixedWebSites;
		}
		int64_t GetWebSiteModID(KNetworkProviderID index) const;
		bool HasWebSite(KNetworkProviderID index) const;
		KLabeledValue GetWebSite(KNetworkProviderID index) const;
		void SetWebSite(KNetworkProviderID index, int64_t modID);

		const KxStringVector& GetTags() const
		{
			return m_Tags;
		}
		KxStringVector& GetTags()
		{
			return m_Tags;
		}
		void ToggleTag(const wxString& value, bool set);
};
