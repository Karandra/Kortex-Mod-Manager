#include "stdafx.h"
#include "KPackageManager.h"
#include "Archive/KArchive.h"
#include "GameInstance/KGameInstance.h"
#include "ModManager/KModManager.h"
#include "PluginManager/KPluginManager.h"
#include "GameInstance/Config/KPackageManagerConfig.h"
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxTaskDialog.h>

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
		return KModManager::GetDispatcher().ResolveLocationPath(path);
	}
}

KPPReqState KPackageManager::CheckRequirementState(const KPPRRequirementEntry* entry)
{
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
				bool isActive = KModManager::Get().IsModActive(entry->GetID());
				return (KPPReqState)(objectFunc == KPPR_OBJFUNC_MOD_ACTIVE ? isActive : !isActive);
			}
			return KPPReqState::False;
		}
		case KPPR_OBJFUNC_PLUGIN_ACTIVE:
		case KPPR_OBJFUNC_PLUGIN_INACTIVE:
		{
			if (!entry->GetObject().IsEmpty())
			{
				KPluginManager* manager = KPluginManager::GetInstance();
				if (manager)
				{
					manager->LoadIfNeeded();
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
	const KModEntry* modEntry = KModManager::Get().FindModByID(entry->GetID());
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
	wxString stdReqsFilePath = wxString::Format("%s\\PackageManager\\Requirements\\%s.xml", KApp::Get().GetDataFolder(), KApp::Get().GetCurrentGameID());
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

KPackageManager::KPackageManager()
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
	return FindStdReqirement(KVAR_EXP(KVAR_SCRIPT_EXTENDER_ID));
}

wxString KPackageManager::GetPackagesFolder() const
{
	return m_Options.GetAttribute("PackagesLocation");
}
