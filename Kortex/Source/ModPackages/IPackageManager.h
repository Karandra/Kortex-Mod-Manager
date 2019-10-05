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
			static wxString GetRequirementFilePath(const PackageDesigner::KPPRRequirementEntry* entry);
			static PackageDesigner::KPPReqState CheckRequirementState(const PackageDesigner::KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersionFromBinaryFile(const PackageDesigner::KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersionFromModManager(const PackageDesigner::KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersion(const PackageDesigner::KPPRRequirementEntry* entry);

		protected:
			void LoadRequirementsGroup(PackageDesigner::KPPRRequirementsGroup& group, const KxXMLNode& rootNode);

		public:
			IPackageManager();

		public:
			wxString GetPackagesFolder() const;
			void SetPackagesFolder(const wxString& path) const;

		public:
			virtual const PackageDesigner::KPPRRequirementEntry::Vector& GetStdRequirements() const = 0;
			virtual const PackageDesigner::KPPRRequirementEntry* FindStdReqirement(const wxString& id) const = 0;

			virtual void OnModListMenu(KxMenu& menu, const std::vector<IGameMod*>& selectedMods, IGameMod* focusedMod) = 0;
	};
}
