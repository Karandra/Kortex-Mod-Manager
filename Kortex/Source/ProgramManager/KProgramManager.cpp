#include "stdafx.h"
#include "KProgramManager.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include <Kortex/ModManager.hpp>
#include "KBitmapSize.h"
#include <Kortex/Application.hpp>
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>

using namespace Kortex;

namespace
{
	void LoadProgramsFromXML(KProgramEntry::Vector& programs, const KxXMLNode& rootNode)
	{
		for (KxXMLNode node = rootNode.GetFirstChildElement("Programs").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			KProgramEntry& entry = programs.emplace_back(node);
			if (!entry.IsOK())
			{
				programs.pop_back();
			}
		}
	}

	void InitProcessOptions(KxProcess& process)
	{
		process.SetOptionEnabled(KxPROCESS_WAIT_END, false);
		process.SetOptionEnabled(KxPROCESS_WAIT_INPUT_IDLE, false);
		process.SetOptionEnabled(KxPROCESS_DETACHED, true);
	}
	void InitProcessPaths(KxProcess& process, const wxString& executable, const wxString& arguments = wxEmptyString, const wxString& workingDirectory = wxEmptyString)
	{
		process.SetExecutablePath(executable);
		process.SetWorkingFolder(workingDirectory.IsEmpty() ? executable.BeforeLast(wxS('\\')) : workingDirectory);
		process.SetArguments(arguments);
	}
	void InitProcessPaths(KxProcess& process, const KProgramEntry& runEntry)
	{
		InitProcessPaths(process, runEntry.GetExecutable(), runEntry.GetArguments(), runEntry.GetWorkingDirectory());
	}
}

namespace Kortex
{
	namespace ProgramManager::Internal
	{
		const SimpleManagerInfo TypeInfo("KProgramManager", "ProgramManager.Name");
	}

	void KProgramManager::OnInit()
	{
		LoadUserPrograms();
	}
	void KProgramManager::OnExit()
	{
		SaveUserPrograms();
	}
	void KProgramManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadProgramsFromXML(m_DefaultPrograms, managerNode);
	}

	void KProgramManager::LoadUserPrograms()
	{
		KxFileStream stream(IGameInstance::GetActive()->GetProgramsFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		KxXMLDocument xml(stream);
		LoadProgramsFromXML(m_UserPrograms, xml);
	}
	void KProgramManager::SaveUserPrograms() const
	{
		KxXMLDocument xml;
		KxXMLNode rootNode = xml.NewElement("Programs");

		for (const KProgramEntry& entry: m_UserPrograms)
		{
			entry.Save(rootNode);
		}

		KxFileStream stream(IGameInstance::GetActive()->GetProgramsFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		xml.Save(stream);
	}

	void KProgramManager::LoadEntryImages(KProgramEntry& entry) const
	{
		entry.GetSmallBitmap().SetBitmap(LoadEntryImage(entry, true));
		entry.GetLargeBitmap().SetBitmap(LoadEntryImage(entry, false));
	}
	bool KProgramManager::CheckEntryImages(const KProgramEntry& entry) const
	{
		return entry.GetSmallBitmap().HasBitmap() && entry.GetLargeBitmap().HasBitmap();
	}
	wxBitmap KProgramManager::LoadEntryImage(const KProgramEntry& entry, bool smallBitmap) const
	{
		wxBitmap bitmap;
		wxString path = entry.GetIconPath();

		if (KAux::IsSingleFileExtensionMatches(path, wxS("exe")))
		{
			bitmap = KxShell::GetFileIcon(path, smallBitmap);
		}
		else
		{
			bitmap.LoadFile(path, wxBITMAP_TYPE_ANY);
		}

		if (bitmap.IsOk())
		{
			KBitmapSize bitmapSize;
			if (smallBitmap)
			{
				bitmapSize.FromSystemSmallIcon();
			}
			else
			{
				bitmapSize.FromSystemIcon();
			}

			if (bitmap.GetWidth() != bitmapSize.GetWidth() || bitmap.GetHeight() != bitmapSize.GetHeight())
			{
				bitmap = bitmapSize.ScaleBitmapAspect(bitmap);
			}
		}
		else
		{
			KxFileItem item;
			item.SetName(wxS(".exe"));
			item.SetNormalAttributes();
			bitmap = KxShell::GetFileIcon(item, smallBitmap);
		}
		return bitmap;
	}
	void KProgramManager::OnAddMainMenuItems(KxMenu& menu)
	{
		for (KProgramEntry& entry: m_UserPrograms)
		{
			if (entry.ShouldShowInMainMenu())
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(wxString::Format("%s %s", KTr("Generic.Run"), entry.GetName())));
				item->Enable(entry.CanRunNow());
				item->Bind(KxEVT_MENU_SELECT, [this, &entry](KxMenuEvent& event)
				{
					RunEntry(entry);
				});

				if (!CheckEntryImages(entry))
				{
					LoadEntryImages(entry);
				}
				item->SetBitmap(entry.GetSmallBitmap().GetBitmap());
			}
		}
	}

	KxProcess& KProgramManager::DoCreateProcess(const KProgramEntry& entry) const
	{
		KxProcess* process = new KxProcess();
		InitProcessPaths(*process, entry);
		InitProcessOptions(*process);
		process->SetClientData(const_cast<KProgramEntry*>(&entry));

		return *process;
	}
	int KProgramManager::DoRunProcess(KxProcess& process) const
	{
		KProgramEntry* entry = static_cast<KProgramEntry*>(process.GetClientData());
		entry->OnRun();

		return process.Run(KxPROCESS_RUN_SYNC);
	}
	bool KProgramManager::DoCheckEntry(const KProgramEntry& entry) const
	{
		if (KxFile(entry.GetExecutable()).IsFileExist())
		{
			return true;
		}
		else
		{
			LogEvent(KTr("ProgramManager.FileNotFound") + ":\r\n" + entry.GetExecutable(), LogLevel::Error);
			return false;
		}
	}

	KProgramManager::KProgramManager()
		:ManagerWithTypeInfo(Kortex::KProgramModule::GetInstance())//, m_Options(this, "Options")
	{
	}
	KProgramManager::~KProgramManager()
	{
	}

	void KProgramManager::Save() const
	{
		SaveUserPrograms();
	}
	void KProgramManager::Load()
	{
		LoadUserPrograms();
	}
	void KProgramManager::LoadDefaultPrograms()
	{
		for (const KProgramEntry& entry: m_DefaultPrograms)
		{
			m_UserPrograms.emplace_back(entry);
		}
	}

	KxProcess& KProgramManager::CreateProcess(const KProgramEntry& entry) const
	{
		return DoCreateProcess(entry);
	}
	void KProgramManager::DestroyProcess(KxProcess& process)
	{
		delete &process;
	}
	int KProgramManager::RunProcess(KxProcess& process) const
	{
		return DoRunProcess(process);
	}
	int KProgramManager::RunEntry(const KProgramEntry& entry) const
	{
		if (DoCheckEntry(entry))
		{
			KxProcess& process = DoCreateProcess(entry);
			return DoRunProcess(process);
		}
		return -1;
	}
}

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo ProgramModuleTypeInfo("Programs", "ProgramsModule.Name", "2.0", KIMG_APPLICATION_RUN);
	}

	void KProgramModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
	}
	void KProgramModule::OnInit()
	{
	}
	void KProgramModule::OnExit()
	{
	}

	KProgramModule::KProgramModule()
		:ModuleWithTypeInfo(Disposition::Global)
	{
	}
}
