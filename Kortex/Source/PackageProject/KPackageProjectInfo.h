#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "Utility/KLabeledValue.h"
#include "Network/ModSourceStore.h"
#include "GameMods/ModTagStore.h"
#include "GameMods/TagManager/DefaultTag.h"

namespace Kortex::PackageProject
{
	class InfoSection: public ProjectSection
	{
		private:
			wxString m_Name;
			wxString m_TranslatedName;
			wxString m_Version;
			wxString m_Author;
			wxString m_Translator;
			wxString m_Description;
			KLabeledValue::Vector m_CustomFields;
			KLabeledValue::Vector m_Documents;
			KLabeledValue::Vector m_WebSites;
			ModSourceStore m_ModSourceStore;
			ModTagStore m_TagStore;
	
		public:
			InfoSection(ModPackageProject& project);
			virtual ~InfoSection();
	
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
			
			const KLabeledValue::Vector& GetCustomFields() const
			{
				return m_CustomFields;
			}
			KLabeledValue::Vector& GetCustomFields()
			{
				return m_CustomFields;
			}
			
			const KLabeledValue::Vector& GetDocuments() const
			{
				return m_Documents;
			}
			KLabeledValue::Vector& GetDocuments()
			{
				return m_Documents;
			}
			
			const ModSourceStore& GetModSourceStore() const
			{
				return m_ModSourceStore;
			}
			ModSourceStore& GetModSourceStore()
			{
				return m_ModSourceStore;
			}
			
			const ModTagStore& GetTagStore() const
			{
				return m_TagStore;
			}
			ModTagStore& GetTagStore()
			{
				return m_TagStore;
			}
	};
}
