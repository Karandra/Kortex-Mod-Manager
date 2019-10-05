#include "stdafx.h"
#include "IPackageManager.h"
#include "ModPackagesModule.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Common/Packages.hpp>
#include "PackageProject/KPackageProject.h"
#include "PackageCreator/KPackageCreatorWorkspace.h"
#include "Archive/KArchive.h"
#include "Utility/KAux.h"
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex
{
	namespace PackageDesigner::Internal
	{
		const SimpleManagerInfo TypeInfo("PackageManager", "PackageManager.Name");
	}

	KxStringVector IPackageManager::GetSuppoptedExtensions()
	{
		return {wxS("kmp"), wxS("smi"), wxS("fomod"), wxS("7z"), wxS("zip")};
	}
	wxString IPackageManager::GetSuppoptedExtensionsFilter()
	{
		return KAux::MakeExtensionsFilter(GetSuppoptedExtensions());
	}
	void IPackageManager::ExtractAcrhiveWithProgress(wxWindow* window, const wxString& filePath, const wxString& outPath)
	{
		auto thread = new KOperationWithProgressDialog<KxArchiveEvent>(true, wxGetTopLevelParent(window));
		thread->OnRun([filePath, outPath](KOperationWithProgressBase* self)
		{
			KArchive archive(filePath);
			self->LinkHandler(&archive, KxEVT_ARCHIVE);
			const bool success = archive.ExtractAll(outPath);

			// Set extraction successfulness status
			self->SetClientData(success ? reinterpret_cast<void*>(1) : nullptr);
		});
		thread->OnEnd([window, outPath](KOperationWithProgressBase* self)
		{
			// Show warning message if something went wrong
			if (self->GetClientData() == nullptr)
			{
				KxTaskDialog dialog(window, KxID_NONE, KTrf("InstallWizard.LoadFailed.Caption", outPath), KTr("InstallWizard.LoadFailed.Message"), KxBTN_OK, KxICON_ERROR);
				dialog.ShowModal();
			}
		});
		thread->SetDialogCaption(KTr("ModManager.ExtractingPackageFiles"));
		thread->Run();
	}

	bool IPackageManager::IsPathAbsolute(const wxString& path)
	{
		return path.IsEmpty() || (path.Length() >= 2 && path[1] == wxS(':'));
	}
	wxString IPackageManager::GetRequirementFilePath(const PackageDesigner::KPPRRequirementEntry* entry)
	{
		wxString path = KVarExp(entry->GetObject());
		if (IsPathAbsolute(path))
		{
			// Is object is absolute file path, return it as is.
			return path;
		}
		else
		{
			return IModDispatcher::GetInstance()->ResolveLocationPath(path);
		}
	}

	PackageDesigner::KPPReqState IPackageManager::CheckRequirementState(const PackageDesigner::KPPRRequirementEntry* entry)
	{
		using namespace PackageDesigner;

		KPPRObjectFunction objectFunc = entry->GetObjectFunction();
		switch (objectFunc)
		{
			case KPPR_OBJFUNC_NONE:
			{
				return KPPReqState::True;
			}
			case KPPR_OBJFUNC_MOD_ACTIVE:
			case KPPR_OBJFUNC_MOD_INACTIVE:
			{
				if (!entry->GetID().IsEmpty())
				{
					bool isActive = IModManager::GetInstance()->IsModActive(entry->GetID());
					return (KPPReqState)(objectFunc == KPPR_OBJFUNC_MOD_ACTIVE ? isActive : !isActive);
				}
				return KPPReqState::False;
			}
			case KPPR_OBJFUNC_PLUGIN_ACTIVE:
			case KPPR_OBJFUNC_PLUGIN_INACTIVE:
			{
				if (!entry->GetObject().IsEmpty())
				{
					Kortex::IPluginManager* manager = Kortex::IPluginManager::GetInstance();
					if (manager)
					{
						if (!manager->HasPlugins())
						{
							manager->Load();
						}

						bool isActive = manager->IsPluginActive(entry->GetObject());
						return (KPPReqState)(objectFunc == KPPR_OBJFUNC_PLUGIN_ACTIVE ? isActive : !isActive);
					}
					return KPPReqState::Unknown;
				}
				return KPPReqState::False;
			}
			case KPPR_OBJFUNC_FILE_EXIST:
			case KPPR_OBJFUNC_FILE_NOT_EXIST:
			{
				if (!entry->GetObject().IsEmpty())
				{
					bool isExist = KxFile(GetRequirementFilePath(entry)).IsExist();
					return (KPPReqState)(objectFunc == KPPR_OBJFUNC_FILE_EXIST ? isExist : !isExist);
				}
				return KPPReqState::False;
			}
		};
		return KPPReqState::Unknown;
	}
	KxVersion IPackageManager::GetRequirementVersionFromBinaryFile(const PackageDesigner::KPPRRequirementEntry* entry)
	{
		KxVersion version;

		KxFile file(GetRequirementFilePath(entry));
		if (file.IsFileExist())
		{
			KxLibraryVersionInfo versionInfo = file.GetVersionInfo();
			if (versionInfo.IsOK())
			{
				auto TryWith = [&versionInfo](const wxString& name) -> KxVersion
				{
					KxVersion version(versionInfo.GetString(name));
					if (version.IsOK())
					{
						return version;
					}
					return KxNullVersion;
				};

				version = TryWith(entry->GetBinaryVersionKind());
				if (!version.IsOK())
				{
					version = TryWith("ProductVersion");
					if (!version.IsOK())
					{
						version = TryWith("FileVersion");
						if (!version.IsOK())
						{
							version = TryWith("ProductVersionString");
							if (!version.IsOK())
							{
								version = TryWith("FileVersionString");
							}
						}
					}
				}
			}

			// No versions available, use file modification time
			if (!version.IsOK())
			{
				version = file.GetFileTime(KxFILETIME_MODIFICATION);
			}
		}
		return version;
	}
	KxVersion IPackageManager::GetRequirementVersionFromModManager(const PackageDesigner::KPPRRequirementEntry* entry)
	{
		const IGameMod* modEntry = IModManager::GetInstance()->FindModByID(entry->GetID());
		if (modEntry)
		{
			return modEntry->GetVersion();
		}
		return KxNullVersion;
	}
	KxVersion IPackageManager::GetRequirementVersion(const PackageDesigner::KPPRRequirementEntry* entry)
	{
		KxVersion modVersion = GetRequirementVersionFromModManager(entry);
		return modVersion.IsOK() ? modVersion : GetRequirementVersionFromBinaryFile(entry);
	}

	void IPackageManager::LoadRequirementsGroup(PackageDesigner::KPPRRequirementsGroup& group, const KxXMLNode& rootNode)
	{
		using namespace PackageDesigner;

		for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			auto& entry = group.GetEntries().emplace_back(std::make_unique<KPPRRequirementEntry>(KPPR_TYPE_SYSTEM));
			entry->SetID(KVarExp(node.GetAttribute("ID")));
			entry->SetCategory(KVarExp(node.GetAttribute("Category")));
			entry->SetName(KVarExp(node.GetFirstChildElement("Name").GetValue()));

			// Object
			KxXMLNode objectNode = node.GetFirstChildElement("Object");
			entry->SetObject(objectNode.GetValue());
			entry->SetObjectFunction(KPackageProjectRequirements::StringToObjectFunction(objectNode.GetAttribute("Function")));

			// Version
			KxXMLNode versionNode = node.GetFirstChildElement("Version");
			entry->SetRequiredVersion(versionNode.GetValue());
			entry->SetRVFunction(KPackageProject::StringToOperator(versionNode.GetAttribute("Function"), false, KPackageProjectRequirements::ms_DefaultVersionOperator));
			entry->SetBinaryVersionKind(versionNode.GetAttribute("BinaryVersionKind"));

			// Description
			entry->SetDescription(node.GetFirstChildElement("Description").GetValue());

			// Dependencies
			KAux::LoadStringArray(entry->GetDependencies(), node.GetFirstChildElement("Dependencies"));
		}
	}

	IPackageManager::IPackageManager()
		:ManagerWithTypeInfo(ModPackagesModule::GetInstance())
	{
	}

	wxString IPackageManager::GetPackagesFolder() const
	{
		using namespace Application;
		return GetAInstanceOption(OName::Packages).GetAttribute(OName::Location);
	}
	void IPackageManager::SetPackagesFolder(const wxString& path) const
	{
		using namespace Application;
		GetAInstanceOption(OName::Packages).SetAttribute(OName::Location, path);
	}
}
