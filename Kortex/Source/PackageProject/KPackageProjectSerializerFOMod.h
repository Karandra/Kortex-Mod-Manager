#pragma once
#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include "KPackageProjectDefs.h"
#include "KPackageProjectComponents.h"
#include <KxFramework/KxXML.h>

namespace Kortex
{
	class KPackageProject;
}
namespace Kortex::PackageProject
{
	class KPPFFileEntry;
	class KPPRRequirementsGroup;
}

namespace Kortex::PackageProject
{
	class KPackageProjectSerializerFOMod: public KPackageProjectSerializer
	{
		private:
			using FilePriorityArray = std::vector<std::pair<KPPFFileEntry*, int64_t>>;
			
		private:
			wxString m_InfoXML;
			wxString m_ModuleConfigXML;
			wxString m_ProjectFolder;
	
			KPackageProject* m_ProjectLoad = nullptr;
			const KPackageProject* m_ProjectSave = nullptr;
			KxXMLDocument m_XML;
			bool m_ExportToNativeFormat = false;
	
			wxString m_EffectiveArchiveRoot;
			bool m_HasDataFolderAsRoot = false;
			bool m_IsMorrowind = false;
	
		private:
			bool IsRootPathHandlingNeeded() const
			{
				return m_HasDataFolderAsRoot || m_IsMorrowind;
			}
			wxString GetDataFolderName(bool withSeparator) const;
			wxString MakeProjectPath(const wxString& path) const;
			PackageProject::KPPCSelectionMode ConvertSelectionMode(const wxString& mode) const;
			wxString ConvertSelectionMode(PackageProject::KPPCSelectionMode mode) const;
			template<class T> void UniqueStringArray(T& array)
			{
				auto it = std::unique(array.begin(), array.end());
				array.erase(it, array.end());
			}
	
		private:
			/* Structurize */
			void ReadInfo();
	
			void ReadInstallSteps();
			void ReadConditionalSteps(const KxXMLNode& stepsArrayNode);
			FilePriorityArray ReadFileData(const KxXMLNode& filesArrayNode, KPPCEntry* entry = nullptr);
			void UniqueFileData();
			void UniqueImages();
	
			/* Serialize */
			void WriteInfo();
			void WriteSites(KxXMLNode& infoNode, KxXMLNode& sitesNode);
	
			void WriteInstallSteps();
			void WriteConditionalSteps(KxXMLNode& stepsArrayNode);
			void WriteFileData(KxXMLNode& node, const KxStringVector& files, bool alwaysInstall = false);
			void WriteRequirements(KxXMLNode& node, const KxStringVector& requiremetSets);
	
		private:
			void InitDataFolderInfo();
			void Init();
	
		public:
			KPackageProjectSerializerFOMod(const wxString& projectFolder = wxEmptyString);
			KPackageProjectSerializerFOMod(const wxString& sInfoXML, const wxString& moduleConfigXML, const wxString& projectFolder = wxEmptyString);
	
		public:
			void Serialize(const KPackageProject* project) override;
			void Structurize(KPackageProject* project) override;
	
			void ExportToNativeFormat(bool value)
			{
				m_ExportToNativeFormat = value;
				SetPackageDataRoot(m_ExportToNativeFormat ? GetDefaultFOModRoot() : GetDefaultKMPRoot());
			}
			void SetEffectiveArchiveRoot(const wxString& path)
			{
				m_EffectiveArchiveRoot = path;
				if (!m_EffectiveArchiveRoot.IsEmpty() && m_EffectiveArchiveRoot.Last() == '\\')
				{
					m_EffectiveArchiveRoot.RemoveLast(1);
				}
			}
			
			const wxString& GetInfoXML() const
			{
				return m_InfoXML;
			}
			const wxString& GetModuleConfigXML() const
			{
				return m_ModuleConfigXML;
			}
	};
}
