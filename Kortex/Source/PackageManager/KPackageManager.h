#pragma once
#include "stdafx.h"
#include "Application/IModule.h"
#include "Application/IManager.h"
#include "PackageProject/KPackageProjectDefs.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxVersion.h>
#include <KxFramework/KxSingleton.h>
class KxXMLNode;

namespace Kortex
{
	namespace PackageManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class KPackageManager:
		public ManagerWithTypeInfo<IManager, SimpleManagerInfo, PackageManager::Internal::TypeInfo>,
		public KxSingletonPtr<KPackageManager>
	{
		friend class KPackageModule;

		public:
			static const KxStringVector& GetSuppoptedExtensions();
			static const wxString& GetSuppoptedExtensionsFilter();
			static void ExtractAcrhiveThreaded(wxWindow* window, const wxString& filePath, const wxString& outPath);

		public:
			static bool IsPathAbsolute(const wxString& path);
			static wxString GetRequirementFilePath(const KPPRRequirementEntry* entry);
			static KPPReqState CheckRequirementState(const KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersionFromBinaryFile(const KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersionFromModManager(const KPPRRequirementEntry* entry);
			static KxVersion GetRequirementVersion(const KPPRRequirementEntry* entry);

		private:
			KPPRRequirementsGroup m_StdEntries;

		private:
			void LoadStdRequirements();

			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

		public:
			KPackageManager();
			virtual ~KPackageManager();

		public:
			const KPPRRequirementEntry::Vector& GetStdRequirements() const
			{
				return m_StdEntries.GetEntries();
			}
			const KPPRRequirementEntry* FindStdReqirement(const wxString& id) const
			{
				return m_StdEntries.FindEntry(id);
			}
			const KPPRRequirementEntry* GetScriptExtenderRequirement() const;
			bool HasScriptExtender() const
			{
				return GetScriptExtenderRequirement() != nullptr;
			}
			bool IsStdReqirement(const wxString& id) const
			{
				return FindStdReqirement(id) != nullptr;
			}

			wxString GetPackagesFolder() const;
	};
}

namespace Kortex
{
	namespace Internal
	{
		extern const SimpleModuleInfo PackagesModuleTypeInfo;
	};

	class KPackageModule:
		public ModuleWithTypeInfo<IModule, SimpleModuleInfo, Internal::PackagesModuleTypeInfo>,
		public KxSingletonPtr<KPackageModule>
	{
		private:
			KPackageManager m_PackageManager;

		protected:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

		public:
			KPackageModule();

		public:
			virtual ManagerRefVector GetManagers() override
			{
				return ToManagersList(m_PackageManager);
			}
	};
}