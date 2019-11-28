#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "PackageProject/KPackageProjectDefs.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxComponentSystem.h>
class KxMenu;

namespace Kortex
{
	class IGameMod;
}

namespace Kortex
{
	namespace PackageDesigner::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IPackageManager:
		public ManagerWithTypeInfo<IManager, PackageDesigner::Internal::TypeInfo>,
		public KxComponentContainer,
		public KxSingletonPtr<IPackageManager>
	{
		friend class ModPackagesModule;

		public:
			static KxStringVector GetSuppoptedExtensions();
			static wxString GetSuppoptedExtensionsFilter();
			static void ExtractAcrhiveWithProgress(wxWindow* window, const wxString& filePath, const wxString& outPath);

		public:
			static bool IsPathAbsolute(const wxString& path);
			static wxString GetRequirementFilePath(const PackageProject::KPPRRequirementEntry* entry);
			static PackageProject::KPPReqState CheckRequirementState(const PackageProject::KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersionFromBinaryFile(const PackageProject::KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersionFromModManager(const PackageProject::KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersion(const PackageProject::KPPRRequirementEntry* entry);

		protected:
			void LoadRequirementsGroup(PackageProject::KPPRRequirementsGroup& group, const KxXMLNode& rootNode);

		public:
			IPackageManager();

		public:
			wxString GetPackagesFolder() const;
			void SetPackagesFolder(const wxString& path) const;

		public:
			virtual const PackageProject::KPPRRequirementEntry::Vector& GetStdRequirements() const = 0;
			virtual const PackageProject::KPPRRequirementEntry* FindStdReqirement(const wxString& id) const = 0;

			virtual void OnModListMenu(KxMenu& menu, const std::vector<IGameMod*>& selectedMods, IGameMod* focusedMod) = 0;
	};
}
