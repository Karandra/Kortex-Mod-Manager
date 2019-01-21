#include "stdafx.h"
#include "KPackageManager.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Common/Packages.hpp>
#include "PackageProject/KPackageProject.h"
#include "Archive/KArchive.h"
#include "Utility/KAux.h"
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxTaskDialog.h>

namespace Kortex
{
	namespace PackageManager::Internal
	{
		const SimpleManagerInfo TypeInfo("PackageManager", "PackageManager.Name");
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
			self->SetClientData(ok ? (void*)1 : nullptr);
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

	bool KPackageManager::IsPathAbsolute(const wxString& path)
	{
		return path.IsEmpty() || (path.Length() >= 2 && path[1] == ':');
	}
	wxString KPackageManager::GetRequirementFilePath(const KPPRRequirementEntry* entry)
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
		const IGameMod* modEntry = IModManager::GetInstance()->FindModByID(entry->GetID());
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
		wxString stdReqsFilePath = KxFormat("%1\\PackageManager\\Requirements\\%2.xml").arg(IApplication::GetInstance()->GetDataFolder()).arg(IGameInstance::GetActive()->GetGameID());
		KxFileStream file(stdReqsFilePath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		KxXMLDocument xml(file);

		for (KxXMLNode entryNode = xml.GetFirstChildElement("Requirements").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			auto& entry = m_StdEntries.GetEntries().emplace_back(new KPPRRequirementEntry(KPPR_TYPE_SYSTEM));
			entry->SetID(KVarExp(entryNode.GetAttribute("ID")));
			entry->SetCategory(KVarExp(entryNode.GetAttribute("Category")));
			entry->SetName(KVarExp(entryNode.GetFirstChildElement("Name").GetValue()));

			// Object
			KxXMLNode objectNode = entryNode.GetFirstChildElement("Object");
			entry->SetObject(objectNode.GetValue());
			entry->SetObjectFunction(KPackageProjectRequirements::StringToObjectFunction(objectNode.GetAttribute("Function")));

			// Version
			KxXMLNode versionNode = entryNode.GetFirstChildElement("Version");
			entry->SetRequiredVersion(versionNode.GetValue());
			entry->SetRVFunction(KPackageProject::StringToOperator(versionNode.GetAttribute("Function"), false, KPackageProjectRequirements::ms_DefaultVersionOperator));
			entry->SetBinaryVersionKind(versionNode.GetAttribute("BinaryVersionKind"));

			// Description
			entry->SetDescription(entryNode.GetFirstChildElement("Description").GetValue());

			// Dependencies
			KAux::LoadStringArray(entry->GetDependencies(), entryNode.GetFirstChildElement("Dependencies"));
		}
	}

	void KPackageManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadStdRequirements();
	}
	void KPackageManager::OnInit()
	{
	}
	void KPackageManager::OnExit()
	{
	}

	KPackageManager::KPackageManager()
		:ManagerWithTypeInfo(Kortex::KPackageModule::GetInstance())//, m_Options(this, "General")
	{
	}
	KPackageManager::~KPackageManager()
	{
	}

	const KPPRRequirementEntry* KPackageManager::GetScriptExtenderRequirement() const
	{
		return FindStdReqirement(GetVariable(Variables::KVAR_SCRIPT_EXTENDER_ID));
	}
	wxString KPackageManager::GetPackagesFolder() const
	{
		using namespace Application;

		return GetAInstanceOption(OName::Package).GetAttribute(OName::Location);
	}
}

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo PackagesModuleTypeInfo("Packages", "PackagesModule.Name", "1.3.1", KIMG_BOX);
	}

	void KPackageModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
	}
	void KPackageModule::OnInit()
	{
	}
	void KPackageModule::OnExit()
	{
	}

	KPackageModule::KPackageModule()
		:ModuleWithTypeInfo(Disposition::Global)
	{
	}
}
