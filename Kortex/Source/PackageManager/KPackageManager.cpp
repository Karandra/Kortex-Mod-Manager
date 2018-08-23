#include "stdafx.h"
#include "KPackageManager.h"
#include "Archive/KArchive.h"
#include "Profile/KProfile.h"
#include "ModManager/KModManager.h"
#include "PluginManager/KPluginManager.h"
#include "Profile/KPackageManagerConfig.h"
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxTaskDialog.h>

KxSingletonPtr_Define(KPackageManager);

wxCheckBoxState KPackageManager::CheckRequirementStateStep1(const KPPRRequirementEntry* entry, KPPRObjectFunction nObjFunc)
{
	switch (nObjFunc)
	{
		case KPPR_OBJFUNC_NONE:
		{
			return wxCHK_CHECKED;
		}
		case KPPR_OBJFUNC_MOD_ACTIVE:
		case KPPR_OBJFUNC_MOD_INACTIVE:
		{
			bool isActive = KModManager::Get().IsModActive(entry->GetID());
			return (wxCheckBoxState)(nObjFunc == KPPR_OBJFUNC_MOD_ACTIVE ? isActive : !isActive);
		}
		case KPPR_OBJFUNC_PLUGIN_ACTIVE:
		case KPPR_OBJFUNC_PLUGIN_INACTIVE:
		{
			KPluginManager* manager = KPluginManager::GetInstance();
			if (manager)
			{
				manager->LoadIfNeeded();
				bool isActive = manager->IsPluginActive(entry->GetObject());
				return (wxCheckBoxState)(nObjFunc == KPPR_OBJFUNC_PLUGIN_ACTIVE ? isActive : !isActive);
			}
			return wxCHK_UNDETERMINED;
		}
		case KPPR_OBJFUNC_FILE_EXIST:
		case KPPR_OBJFUNC_FILE_NOT_EXIST:
		{
			bool isExist = KxFile(GetRequirementFilePath(entry)).IsExist();
			return (wxCheckBoxState)(nObjFunc == KPPR_OBJFUNC_FILE_EXIST ? isExist : !isExist);
		}
	};
	return wxCHK_UNDETERMINED;
}

KPackageManager& KPackageManager::Get()
{
	return *KPackageManagerConfig::GetInstance()->GetManager();
}
const KxStringVector& KPackageManager::GetSuppoptedExtensions()
{
	static const KxStringVector ms_SupportedExtensions = {"kmp", "smi", "fomod", "7z", "zip"};
	return ms_SupportedExtensions;
}
const wxString& KPackageManager::GetSuppoptedExtensionsFilter()
{
	static const wxString ms_SupportedExtensionsFilter = KAux::MakeExtensionsFilter(GetSuppoptedExtensions());
	return ms_SupportedExtensionsFilter;
}
void KPackageManager::ExtractAcrhiveThreaded(wxWindow* window, const wxString& filePath, const wxString& outPath)
{
	auto thread = new KOperationWithProgressDialog<KxArchiveEvent>(true, wxGetTopLevelParent(window));
	thread->OnRun([filePath, outPath](KOperationWithProgressBase* self)
	{
		KArchive archive(filePath);
		self->LinkHandler(&archive, KxEVT_ARCHIVE);
		bool ok = archive.ExtractAll(outPath);

		// Set extraction successfulness status
		self->SetClientData(ok ? (void*)1 : NULL);
	});
	thread->OnEnd([window, outPath](KOperationWithProgressBase* self)
	{
		// Show warning message if something went wrong
		if (self->GetClientData() == NULL)
		{
			KxTaskDialog dialog(window, KxID_NONE, TF("InstallWizard.LoadFailed.Caption").arg(outPath), T("InstallWizard.LoadFailed.Message"), KxBTN_OK, KxICON_ERROR);
			dialog.ShowModal();
		}
	});
	thread->SetDialogCaption(T("ModManager.ExtractingPackageFiles"));
	thread->Run();
}

bool KPackageManager::IsPathAbsolute(const wxString& path)
{
	return path.IsEmpty() || (path.Length() >= 2 && path[1] == ':');
}
wxString KPackageManager::GetRequirementFilePath(const KPPRRequirementEntry* entry)
{
	wxString path = V(entry->GetObject());
	if (IsPathAbsolute(path))
	{
		// Is object is absolute file path, return it as is.
		return path;
	}
	else
	{
		return KModManager::GetDispatcher().GetTargetPath(path);
	}
}

wxCheckBoxState KPackageManager::CheckRequirementState(const KPPRRequirementEntry* entry, KPPRObjectFunction* finalObjFunc)
{
	KPPRObjectFunction nObjectFunction = entry->GetObjectFunction();
	wxCheckBoxState result = CheckRequirementStateStep1(entry, nObjectFunction);

	KxUtility::SetIfNotNull(finalObjFunc, nObjectFunction);
	if (result == wxCHK_UNCHECKED)
	{
		switch (nObjectFunction)
		{
			case KPPR_OBJFUNC_PLUGIN_ACTIVE:
			case KPPR_OBJFUNC_FILE_EXIST:
			{
				KxUtility::SetIfNotNull(finalObjFunc, KPPR_OBJFUNC_MOD_ACTIVE);
				return CheckRequirementStateStep1(entry, KPPR_OBJFUNC_MOD_ACTIVE);
			}
			case KPPR_OBJFUNC_PLUGIN_INACTIVE:
			case KPPR_OBJFUNC_FILE_NOT_EXIST:
			{
				KxUtility::SetIfNotNull(finalObjFunc, KPPR_OBJFUNC_MOD_INACTIVE);
				return CheckRequirementStateStep1(entry, KPPR_OBJFUNC_MOD_INACTIVE);
			}

			case KPPR_OBJFUNC_MOD_ACTIVE:
			{
				KxUtility::SetIfNotNull(finalObjFunc, KPPR_OBJFUNC_PLUGIN_ACTIVE);
				return CheckRequirementStateStep1(entry, KPPR_OBJFUNC_PLUGIN_ACTIVE);
			}
			case KPPR_OBJFUNC_MOD_INACTIVE:
			{
				KxUtility::SetIfNotNull(finalObjFunc, KPPR_OBJFUNC_PLUGIN_INACTIVE);
				return CheckRequirementStateStep1(entry, KPPR_OBJFUNC_PLUGIN_INACTIVE);
			}
		};
		return wxCHK_UNDETERMINED;
	}
	return result;
}
KxVersion KPackageManager::GetRequirementVersionFromBinaryFile(const KPPRRequirementEntry* entry)
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
KxVersion KPackageManager::GetRequirementVersionFromModManager(const KPPRRequirementEntry* entry)
{
	const KModEntry* modEntry = KModManager::Get().FindMod(entry->GetID());
	if (modEntry)
	{
		return modEntry->GetVersion();
	}
	return KxNullVersion;
}
KxVersion KPackageManager::GetRequirementVersion(const KPPRRequirementEntry* entry)
{
	KxVersion modVersion = GetRequirementVersionFromModManager(entry);
	return modVersion.IsOK() ? modVersion : GetRequirementVersionFromBinaryFile(entry);
}

void KPackageManager::LoadStdRequirements()
{
	wxString stdReqsFilePath = wxString::Format("%s\\PackageManager\\Requirements\\%s.xml", KApp::Get().GetDataFolder(), KApp::Get().GetCurrentTemplateID());
	KxFileStream file(stdReqsFilePath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxXMLDocument xml(file);

	for (KxXMLNode entryNode = xml.GetFirstChildElement("Requirements").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		auto& entry = m_StdEntries.GetEntries().emplace_back(new KPPRRequirementEntry(KPPR_TYPE_SYSTEM));
		entry->SetID(V(entryNode.GetAttribute("ID")));
		entry->SetCategory(V(entryNode.GetAttribute("Category")));
		entry->SetName(V(entryNode.GetFirstChildElement("Name").GetValue()));

		// Object
		KxXMLNode objectNode = entryNode.GetFirstChildElement("Object");
		entry->SetObject(objectNode.GetValue());
		entry->SetObjectFunction(KPackageProjectRequirements::StringToObjectFunction(objectNode.GetAttribute("Function")));

		// Version
		KxXMLNode versionNode = entryNode.GetFirstChildElement("Version");
		entry->SetRequiredVersion(versionNode.GetValue());
		entry->SetRVFunction(KPackageProjectRequirements::StringToOperator(versionNode.GetAttribute("Function"), false));
		entry->SetBinaryVersionKind(versionNode.GetAttribute("BinaryVersionKind"));

		// Description
		entry->SetDescription(entryNode.GetFirstChildElement("Description").GetValue());

		// Dependencies
		KAux::LoadStringArray(entry->GetDependencies(), entryNode.GetFirstChildElement("Dependencies"));
	}
}

KPackageManager::KPackageManager(const KxXMLNode& configNode)
	:m_Options(this, "General")
{
	LoadStdRequirements();
}
KPackageManager::~KPackageManager()
{
}

wxString KPackageManager::GetID() const
{
	return "KPackageManager";
}
wxString KPackageManager::GetName() const
{
	return T("PackageManager.Name");
}
wxString KPackageManager::GetVersion() const
{
	return "1.2.2";
}

const KPPRRequirementEntry* KPackageManager::FindScriptExtenderRequirement() const
{
	return FindStdReqirement(V("$(ScriptExtenderNameShort)"));
}

wxString KPackageManager::GetPackagesFolder() const
{
	return m_Options.GetAttribute("PackagesLocation");
}
