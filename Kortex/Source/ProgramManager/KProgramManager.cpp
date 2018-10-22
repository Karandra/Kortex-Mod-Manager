#include "stdafx.h"
#include "KProgramManager.h"
#include "GameInstance/KInstanceManagement.h"
#include "KEvents.h"
#include "UI/KMainWindow.h"
#include "ModManager/KModManager.h"
#include "ModManager/KDispatcher.h"
#include "GameInstance/Config/KProgramManagerConfig.h"
#include "KBitmapSize.h"
#include "KApp.h"
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

void KProgramManager::OnInit()
{
	LoadUserPrograms();
}
void KProgramManager::OnLoadConfig(const KxXMLNode& configNode)
{
	LoadProgramsFromXML(m_DefaultPrograms, configNode);
}
void KProgramManager::LoadUserPrograms()
{
	KxFileStream stream(KGameInstance::GetActive()->GetProgramsFile(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
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

	KxFileStream stream(KGameInstance::GetActive()->GetProgramsFile(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
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
			KxMenuItem* item = menu.Add(new KxMenuItem(wxString::Format("%s %s", T("Generic.Run"), entry.GetName())));
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
		KLogEvent(T("ProgramManager.FileNotFound") + ":\r\n" + entry.GetExecutable(), KLOG_ERROR);
		return false;
	}
}

KProgramManager::KProgramManager()
	:m_Options(this, "Options")
{
}
KProgramManager::~KProgramManager()
{
	SaveUserPrograms();
}

wxString KProgramManager::GetID() const
{
	return "KProgramManager";
}
wxString KProgramManager::GetName() const
{
	return T("ProgramManager.Name");
}
wxString KProgramManager::GetVersion() const
{
	return "1.1";
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
