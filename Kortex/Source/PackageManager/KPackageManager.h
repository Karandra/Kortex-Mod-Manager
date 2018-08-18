#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "PackageProject/KPackageProjectDefs.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include "KProgramOptions.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxVersion.h>
#include <KxFramework/KxSingleton.h>
class KxXMLNode;

class KPackageManager: public KManager, public KxSingletonPtr<KPackageManager>
{
	private:
		static wxCheckBoxState CheckRequirementStateStep1(const KPPRRequirementEntry* entry, KPPRObjectFunction nObjFunc);

	public:
		static KPackageManager& Get();
		static const KxStringVector& GetSuppoptedExtensions();
		static const wxString& GetSuppoptedExtensionsFilter();
		static void ExtractAcrhiveThreaded(wxWindow* window, const wxString& filePath, const wxString& outPath);

	public:
		static bool IsPathAbsolute(const wxString& path);
		static wxString GetRequirementFilePath(const KPPRRequirementEntry* entry);
		static wxString GetPluginRequirementFilePath(const KPPRRequirementEntry* entry);
		static wxCheckBoxState CheckRequirementState(const KPPRRequirementEntry* entry, KPPRObjectFunction* finalObjFunc = NULL);
		static KxVersion GetRequirementVersionFromBinaryFile(const KPPRRequirementEntry* entry);
		static KxVersion GetRequirementVersionFromModManager(const KPPRRequirementEntry* entry);
		static KxVersion GetRequirementVersion(const KPPRRequirementEntry* entry);

	private:
		KProgramOptionUI m_Options;
		KPPRRequirementsGroup m_StdEntries;

	private:
		void LoadStdRequirements();

	public:
		KPackageManager(const KxXMLNode& configNode);
		virtual ~KPackageManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_BOX;
		}

	public:
		KProgramOption& GetOptions()
		{
			return m_Options;
		}

		const KPPRRequirementEntryArray& GetStdRequirements() const
		{
			return m_StdEntries.GetEntries();
		}
		const KPPRRequirementEntry* FindStdReqirement(const wxString& id) const
		{
			return m_StdEntries.FindEntry(id);
		}
		const KPPRRequirementEntry* FindScriptExtenderRequirement() const;
		bool HasScriptExtender() const
		{
			return FindScriptExtenderRequirement() != NULL;
		}
		bool IsStdReqirement(const wxString& id) const
		{
			return FindStdReqirement(id) != NULL;
		}

		wxString GetPackagesFolder() const;
};
