#include "stdafx.h"
#include "KProgramManager.h"
#include "GameInstance/KInstnaceManagement.h"
#include "KEvents.h"
#include "UI/KMainWindow.h"
#include "ModManager/KModManager.h"
#include "ModManager/KDispatcher.h"
#include "GameInstance/Config/KProgramManagerConfig.h"
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

KProgramEntry::KProgramEntry()
{
}
KProgramEntry::KProgramEntry(const KxXMLNode& node)
{
	m_Name = V(node.GetFirstChildElement("Name").GetValue());
	m_IconPath = V(node.GetFirstChildElement("Icon").GetValue());
	m_Executable = V(node.GetFirstChildElement("Executable").GetValue());
	m_Arguments = V(node.GetFirstChildElement("Arguments").GetValue());
	m_WorkingDirectory = V(node.GetFirstChildElement("WorkingDirectory").GetValue());
}

bool KProgramEntry::IsOK() const
{
	return !m_Name.IsEmpty() && !m_Executable.IsEmpty();
}
bool KProgramEntry::IsRequiresVFS() const
{
	return KDispatcher::GetInstance()->ResolveLocation(m_Executable) != NULL;
}

//////////////////////////////////////////////////////////////////////////
wxString KProgramManager::GetKExecutePath()
{
	if (KxSystem::Is64Bit())
	{
		return KApp::Get().GetDataFolder() + "\\VFS\\KExecute x64.exe";
	}
	else
	{
		return KApp::Get().GetDataFolder() + "\\VFS\\KExecute.exe";
	}
}
void KProgramManager::InitKExecute(KxProcess& process, const wxString& executable, const wxString& arguments, const wxString& workingDirectory)
{
	process.SetExecutablePath(executable);
	process.SetWorkingFolder(workingDirectory.IsEmpty() ? executable.BeforeLast('\\') : workingDirectory);
	process.SetArguments(arguments);
}
void KProgramManager::InitKExecute(KxProcess& process, const KProgramEntry& runEntry)
{
	return InitKExecute(process, runEntry.GetExecutable(), runEntry.GetArguments(), runEntry.GetWorkingDirectory());
}

void KProgramManager::OnInit()
{
	LoadProgramList();
}
wxBitmap KProgramManager::OnQueryItemImage(const KProgramEntry& runEntry) const
{
	wxString iconPath = runEntry.GetIconPath();
	if (iconPath.IsEmpty())
	{
		iconPath = runEntry.GetExecutable();
	}
	if (iconPath.StartsWith(V(KVAR(KVAR_VIRTUAL_GAME_DIR)), &iconPath) && !iconPath.IsEmpty())
	{
		if (iconPath[0] == '\\')
		{
			iconPath.Remove(0, 1);
		}
		if (iconPath.Last() == '\\')
		{
			iconPath.RemoveLast(1);
		}
	}
	iconPath = KModManager::GetDispatcher().ResolveLocationPath(iconPath);

	if (KAux::IsSingleFileExtensionMatches(iconPath, "exe") || KAux::IsSingleFileExtensionMatches(iconPath, "dll"))
	{
		return KxShell::GetFileIcon(iconPath, true);
	}
	else if (!iconPath.IsEmpty())
	{
		const int width = wxSystemSettings::GetMetric(wxSYS_SMALLICON_X);
		const int height = wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y);

		wxImage image(runEntry.GetIconPath(), wxBITMAP_TYPE_ANY);
		if (image.GetSize().GetWidth() > width || image.GetSize().GetHeight() > height)
		{
			image.Rescale(width, height, wxIMAGE_QUALITY_HIGH);
		}
		return wxBitmap(image, 32);
	}
	else
	{
		return KxShell::GetFileIcon(KModManager::GetDispatcher().ResolveLocationPath(runEntry.GetExecutable()), true);
	}
}
void KProgramManager::OnVFSToggled(KVFSEvent& event)
{
	if (event.IsActivated())
	{
		UpdateProgramListImages();
	}
}
void KProgramManager::OnMenuOpen(KxMenuEvent& event)
{
	// Extract icons for profile run entries
	bool isVFSEnabled = KModManager::Get().IsVFSMounted();

	for (KxMenuItem* item: m_MenuItems)
	{
		KProgramEntry* entry = (KProgramEntry*)item->GetClientData();
		if (entry)
		{
			item->Enable(entry->IsRequiresVFS() ? isVFSEnabled : true);

			if (!m_IconsExtracted && !entry->HasBitmap())
			{
				wxBitmap icon = OnQueryItemImage(*entry);
				entry->SetBitmap(icon);
				item->SetBitmap(icon);
			}
		}
	}
	m_IconsExtracted = true;
	event.Skip();
}

void KProgramManager::DoRunEntry(const KProgramEntry& runEntry, KxProgressDialog* dialog, KxProcess** processOut)
{
	auto OnProcessEnd = [this, dialog](wxProcessEvent& event)
	{
		EndRunProcess(dialog, static_cast<KxProcess*>(event.GetEventObject()));
	};

	KxProcess* process = new KxProcess();
	InitKExecute(*process, runEntry);
	process->Bind(wxEVT_END_PROCESS, OnProcessEnd);

	if (processOut)
	{
		*processOut = process;
	}
	else
	{
		process->SetOptionEnabled(KxPROCESS_WAIT_END, false);
		process->Run(KxPROCESS_RUN_SYNC);
	}
}
bool KProgramManager::CheckEntry(const KProgramEntry& runEntry)
{
	if (KxFile(runEntry.GetExecutable()).IsFileExist())
	{
		return true;
	}
	else
	{
		KLogEvent(T("ProgramManager.FileNotFound") + ":\r\n" + runEntry.GetExecutable(), KLOG_ERROR);
		return false;
	}
}

KxProgressDialog* KProgramManager::BeginRunProcess()
{
	KxProgressDialog* dialog = new KxProgressDialog(wxTheApp->GetTopWindow(), KxID_NONE, T("ProgramManager.Executing"), wxDefaultPosition, wxDefaultSize, KxBTN_NONE);
	dialog->Pulse();
	dialog->Bind(wxEVT_CHAR_HOOK, [](wxKeyEvent& event)
	{
		// Don't skip
	});

	KApp::Get().Yield();
	return dialog;
}
void KProgramManager::EndRunProcess(KxProgressDialog* dialog, KxProcess* process)
{
	if (!process->GetRunStatus())
	{
		KLogEvent(T("ProgramManager.RunFailed") + "\r\n\r\n" + KxSystem::GetErrorMessage(process->GetRunLastErrorCode()), KLOG_ERROR, dialog);
	}

	if (dialog)
	{
		dialog->Destroy();
	}
	delete process;
}
void KProgramManager::RunMain(KxProgressDialog* dialog, const KProgramEntry& runEntry)
{
	dialog->SetLabel(T("ProgramManager.ExecutingMain"));
	dialog->Pulse();

	DoRunEntry(runEntry, dialog);
}

void KProgramManager::LoadProgramList()
{
	KxFileStream stream(GetProgramsListFile(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxXMLDocument xml(stream);

	m_ProgramList.clear();
	for (KxXMLNode node = xml.GetFirstChildElement("Programs").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		KProgramEntry& entry = m_ProgramList.emplace_back(KProgramEntry(node));
		if (!entry.IsOK())
		{
			m_ProgramList.pop_back();
		}
	}
}
void KProgramManager::SaveProgramList() const
{
	KxXMLDocument xml;
	KxXMLNode rootNode = xml.NewElement("Programs");

	for (const KProgramEntry& entry: m_ProgramList)
	{
		KxXMLNode node = rootNode.NewElement("Entry");
		node.NewElement("Name").SetValue(entry.GetName());
		if (!entry.GetIconPath().IsEmpty())
		{
			node.NewElement("Icon").SetValue(entry.GetIconPath());
		}
		node.NewElement("Executable").SetValue(entry.GetExecutable());
		node.NewElement("Arguments").SetValue(entry.GetArguments());
		node.NewElement("WorkingDirectory").SetValue(entry.GetWorkingDirectory());
	}

	KxFileStream stream(GetProgramsListFile(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
	xml.Save(stream);
}

KProgramManager::KProgramManager()
	:m_RunOptions(this, "Options")
{
	KEvent::Bind(KEVT_VFS_TOGGLED, &KProgramManager::OnVFSToggled, this);
}
KProgramManager::~KProgramManager()
{
	SaveProgramList();
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
	return "1.0.1";
}

wxString KProgramManager::GetProgramsListFile() const
{
	return KGameInstance::GetActive()->GetProgramsFile();
}
void KProgramManager::UpdateProgramListImages()
{
	for (KProgramEntry& entry: m_ProgramList)
	{
		if (!entry.HasBitmap())
		{
			entry.SetBitmap(OnQueryItemImage(entry));
		}
	}
}

void KProgramManager::OnAddMenuItems(KxMenu* menu)
{
	m_Menu = menu;
	KxIntVector list = m_RunOptions.GetAttributeVectorInt("MainEnabled");
	
	// If the list contains -1 as its first element, fill it with all indexes and save it back to settings
	if (!list.empty() && list.front() == -1)
	{
		list.clear();
		for (size_t i = 0; i < KProgramManagerConfig::GetInstance()->GetProgramsCount(); i++)
		{
			list.emplace_back((int)i);
		}
		m_RunOptions.SetAttributeVectorInt("MainEnabled", list);
	}

	if (!list.empty())
	{
		for (int index: list)
		{
			if (index < KProgramManagerConfig::GetInstance()->GetProgramsCount())
			{
				const KProgramEntry& entry = KProgramManagerConfig::GetInstance()->GetPrograms()[index];

				KxMenuItem* item = menu->Add(new KxMenuItem(wxString::Format("%s %s", T("Generic.Run"), entry.GetName())));
				item->Enable(false);
				item->SetClientData((void*)&entry);
				item->Bind(KxEVT_MENU_SELECT, [this, item, &entry](KxMenuEvent& event)
				{
					OnRunEntry(item, entry);
				});

				m_MenuItems.push_back(item);
			}
		}
	}
	else
	{
		menu->Add(new KxMenuItem(V("<$T(ProgramManager.NoPrograms)>")))->Enable(false);
	}

	m_Menu->Bind(KxEVT_MENU_OPEN, &KProgramManager::OnMenuOpen, this);
}
void KProgramManager::OnRunEntry(KxMenuItem* menuItem, const KProgramEntry& runEntry)
{
	if (CheckEntry(runEntry))
	{
		KxProgressDialog* dialog = BeginRunProcess();
		dialog->Show();

		RunMain(dialog, runEntry);
		dialog->Close();
	}
}

void KProgramManager::Save() const
{
	SaveProgramList();
}
void KProgramManager::Load()
{
	LoadProgramList();
}

bool KProgramManager::RunEntry(const KProgramEntry& runEntry, KxProgressDialog* dialog)
{
	if (CheckEntry(runEntry))
	{
		DoRunEntry(runEntry, dialog);
		return true;
	}
	return false;
}
KxProcess* KProgramManager::RunEntryDelayed(const KProgramEntry& runEntry, KxProgressDialog* dialog)
{
	if (CheckEntry(runEntry))
	{
		KxProcess* process = NULL;
		DoRunEntry(runEntry, dialog, &process);
		return process;
	}
	return NULL;
}
