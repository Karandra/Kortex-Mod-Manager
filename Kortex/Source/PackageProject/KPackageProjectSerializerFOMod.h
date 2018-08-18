#pragma once
#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include "KPackageProjectDefs.h"
#include "KPackageProjectComponents.h"
#include <KxFramework/KxXML.h>
class KPackageProject;
class KPPFFileEntry;
class KPPRRequirementsGroup;

class KPackageProjectSerializerFOMod: public KPackageProjectSerializer
{
	private:
		typedef std::vector<std::pair<KPPFFileEntry*, int64_t>> FilePriorityArray;

	private:
		wxString m_InfoXML;
		wxString m_ModuleConfigXML;
		wxString m_ProjectFolder;

		KPackageProject* m_ProjectLoad = NULL;
		const KPackageProject* m_ProjectSave = NULL;
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
		KPPCSelectionMode ConvertSelectionMode(const wxString& mode) const;
		wxString ConvertSelectionMode(KPPCSelectionMode mode) const;
		KPPOperator DecideOperator(const wxSize& v1, const wxSize& v2 = wxSize(0, 0)) const;
		KxStringVector ConvertTagsArray(const KxStringVector& FOModTags) const;
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
		FilePriorityArray ReadFileData(const KxXMLNode& filesArrayNode, KPPCEntry* entry = NULL);
		KPPOperator ReadCompositeDependencies(const KxXMLNode& dependenciesNode,
											  KPPCFlagEntryArray* conditions = NULL,
											  KPPCEntry* componentsEntry = NULL,
											  KPPRRequirementsGroup** reqGroupOut = NULL,
											  bool alwaysCreateReqGroup = false,
											  const wxString& createdReqGroupID = wxEmptyString
		);
		void UniqueFileData();
		void UniqueImages();

		/* Serialize */
		void WriteInfo();
		void WriteSites(KxXMLNode& infoNode, KxXMLNode& sitesNode);

		void WriteInstallSteps();
		void WriteConditionalSteps(KxXMLNode& stepsArrayNode);
		void WriteFileData(KxXMLNode& node, const KxStringVector& files, bool alwaysInstall = false);
		wxSize WriteRequirements(KxXMLNode& node, const KxStringVector& requiremetSets);

	private:
		void InitDataFolderInfo();
		void Init();

	public:
		KPackageProjectSerializerFOMod(const wxString& projectFolder = wxEmptyString);
		KPackageProjectSerializerFOMod(const wxString& sInfoXML, const wxString& moduleConfigXML, const wxString& projectFolder = wxEmptyString);

	public:
		virtual void Serialize(const KPackageProject* project) override;
		virtual void Structurize(KPackageProject* project) override;

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
