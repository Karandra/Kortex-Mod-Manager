#include "stdafx.h"
#include "KRunManager.h"
#include "Profile/KProfile.h"
#include "Events/KVFSEvent.h"
#include "Events/KLogEvent.h"
#include "UI/KMainWindow.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "Profile/KRunManagerConfig.h"
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

KRunManagerProgram::KRunManagerProgram()
{
}
KRunManagerProgram::KRunManagerProgram(const KxXMLNode& node)
{
	m_RequiresVFS = node.GetAttributeBool("RequiresVFS", true);

	m_Name = V(node.GetFirstChildElement("Name").GetValue());
	m_IconPath = V(node.GetFirstChildElement("Icon").GetValue());
	m_Executable = V(node.GetFirstChildElement("Executable").GetValue());
	m_Arguments = V(node.GetFirstChildElement("Arguments").GetValue());
	m_WorkingDirectory = V(node.GetFirstChildElement("WorkingDirectory").GetValue());
}

bool KRunManagerProgram::IsOK() const
{
	return !m_Name.IsEmpty() && !m_Executable.IsEmpty();
}
bool KRunManagerProgram::CalcRequiresVFS() const
{
	if (!IsRequiresVFS())
	{
		return KxString::ToLower(m_Executable).StartsWith(KxString::ToLower(V(KVAR(KVAR_VIRTUAL_GAME_ROOT))));
	}
	return m_RequiresVFS;
}

//////////////////////////////////////////////////////////////////////////
KRunManager& KRunManager::Get()
{
	return *KApp::Get().GetRunManager();
}
wxString KRunManager::GetProgramsListFile(const wxString& templateID, const wxString& configID)
{
	return KProfile::GetDataPath(templateID, configID) + '\\' + "RunManagerPrograms.xml";
}

const KRunManagerConfig* KRunManager::GetRunConfig()
{
	return KRunManagerConfig::GetInstance();
}
wxString KRunManager::GetKExecutePath()
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
void KRunManager::InitKExecute(KxProcess& process, const wxString& executable, const wxString& arguments, const wxString& workingDirectory)
{
	process.SetExecutablePath(executable);
	process.SetWorkingFolder(workingDirectory.IsEmpty() ? executable.BeforeLast('\\') : workingDirectory);
	process.SetArguments(arguments);
}
void KRunManager::InitKExecute(KxProcess& process, const KRunManagerProgram& runEntry)
{
	return InitKExecute(process, runEntry.GetExecutable(), runEntry.GetArguments(), runEntry.GetWorkingDirectory());
}

wxBitmap KRunManager::OnQueryItemImage(const KRunManagerProgram& runEntry) const
{
	wxString iconPath = runEntry.GetIconPath();
	if (iconPath.IsEmpty())
	{
		iconPath = runEntry.GetExecutable();
	}
	if (iconPath.StartsWith(V(KVAR(KVAR_VIRTUAL_GAME_ROOT)), &iconPath) && !iconPath.IsEmpty())
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
	iconPath = KModManager::GetDispatcher().GetTargetPath(iconPath);

	if (KAux::IsSingleFileExtensionMatches(iconPath, "exe") || KAux::IsSingleFileExtensionMatches(iconPath, "dll"))
	{
		return wxBitmap(KAux::ExtractIconFromBinaryFile(iconPath));
	}
	else if (!iconPath.IsEmpty())
	{
		wxImage image(runEntry.GetIconPath(), wxBITMAP_TYPE_ANY);
		if (image.GetSize().GetWidth() > 16 || image.GetSize().GetHeight() > 16)
		{
			image.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);
		}
		return wxBitmap(image, 32);
	}
	else
	{
		return wxBitmap(KAux::ExtractIconFromBinaryFile(KModManager::GetDispatcher().GetTargetPath(runEntry.GetExecutable())));
	}
}
void KRunManager::OnVFSToggled(KVFSEvent& event)
{
	if (event.IsActivated())
	{
		UpdateProgramListImages();
	}
}
void KRunManager::OnMenuOpen(KxMenuEvent& event)
{
	// Extract icons for profile run entries
	bool isVFSEnabled = KModManager::Get().IsVFSMounted();

	for (KxMenuItem* item: m_MenuItems)
	{
		KRunManagerProgram* entry = (KRunManagerProgram*)item->GetClientData();
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

void KRunManager::DoRunEntry(const KRunManagerProgram& runEntry, KxProgressDialog* dialog, KxProcess** processOut)
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
bool KRunManager::CheckEntry(const KRunManagerProgram& runEntry)
{
	if (KxFile(runEntry.GetExecutable()).IsFileExist())
	{
		return true;
	}
	else
	{
		KLogEvent(T("RunManager.FileNotFound") + ":\r\n" + runEntry.GetExecutable(), KLOG_ERROR);
		return false;
	}
}

KxProgressDialog* KRunManager::BeginRunProcess()
{
	KxProgressDialog* dialog = new KxProgressDialog(wxTheApp->GetTopWindow(), KxID_NONE, T("RunManager.Executing"), wxDefaultPosition, wxDefaultSize, KxBTN_NONE);
	dialog->Pulse();
	dialog->Bind(wxEVT_CHAR_HOOK, [](wxKeyEvent& event)
	{
		// Don't skip
	});

	KApp::Get().Yield();
	return dialog;
}
void KRunManager::EndRunProcess(KxProgressDialog* dialog, KxProcess* process)
{
	if (!process->GetRunStatus())
	{
		KLogEvent(T("RunManager.RunFailed") + "\r\n\r\n" + KxSystem::GetErrorMessage(process->GetRunLastErrorCode()), KLOG_ERROR, dialog);
	}

	if (dialog)
	{
		dialog->Destroy();
	}
	delete process;
}
int KRunManager::GetPreMainInterval() const
{
	return m_RunMainOptions.GetAttributeInt("PreMainInterval", ms_DefaultPreMainInterval);
}
int KRunManager::GetPreMainTimeout() const
{
	return m_RunMainOptions.GetAttributeInt("PreMainTimeout", ms_DefaultPreMainTimeout);
}
KxStringVector KRunManager::CheckPreMain()
{
	KxStringVector missing;
	KxIntVector list = m_RunMainOptions.GetAttributeVectorInt("PreMainSequence");
	if (!list.empty())
	{
		for (int i: list)
		{
			auto entry = m_RunConfig->GetEntryAt(KPRCE_TYPE_PREMAIN, i);
			if (entry)
			{
				if (!KxFile(entry->GetExecutable()).IsFileExist())
				{
					missing.emplace_back(wxString::Format("%s: %s", entry->GetName(), entry->GetExecutable()));
				}
			}
		}
	}
	return missing;
}
void KRunManager::RunPreMain(KxProgressDialog* dialog)
{
	KxIntVector list = m_RunMainOptions.GetAttributeVectorInt("PreMainSequence");
	if (!list.empty())
	{
		dialog->SetLabel(T("RunManager.ExecutingPreMain"));
		dialog->SetValue(0);

		int inverval = GetPreMainInterval();
		for (int i: list)
		{
			auto entry = m_RunConfig->GetEntryAt(KPRCE_TYPE_PREMAIN, i);
			if (entry)
			{
				KxProcess process(entry->GetExecutable(), entry->GetArguments(), V("$(VirtualRoot)"));
				process.SetOptionEnabled(KxPROCESS_WAIT_END, false);
				process.Run(KxPROCESS_RUN_SYNC);

				dialog->SetValue(i, list.size());
				KApp::Get().Yield();
				wxThread::Sleep(inverval);
			}
		}
		wxThread::Sleep(GetPreMainTimeout());
	}
}
void KRunManager::RunMain(KxProgressDialog* dialog, const KRunManagerProgram& runEntry)
{
	dialog->SetLabel(T("RunManager.ExecutingMain"));
	dialog->Pulse();

	DoRunEntry(runEntry, dialog);
}

KRunManager::KRunManager()
	:m_RunConfig(GetRunConfig()), m_RunMainOptions(this, "RunMain")
{
	KApp::Get().SubscribeBroadcasting(this, KEVT_BROADCAST_VFS_TOGGLED);
	Bind(KEVT_BROADCAST_VFS_TOGGLED, &KRunManager::OnVFSToggled, this);

	LoadProgramList();
}
KRunManager::~KRunManager()
{
	SaveProgramList();
}

wxString KRunManager::GetID() const
{
	return "KRunManager";
}
wxString KRunManager::GetName() const
{
	return T("ToolBar.RunManager");
}
wxString KRunManager::GetVersion() const
{
	return "1.0.1";
}

wxString KRunManager::GetProgramsListFile() const
{
	return GetProgramsListFile(KApp::Get().GetCurrentTemplateID(), KApp::Get().GetCurrentConfigID());
}
void KRunManager::UpdateProgramListImages()
{
	for (KRunManagerProgram& entry: m_ProgramList)
	{
		if (!entry.HasBitmap())
		{
			entry.SetBitmap(OnQueryItemImage(entry));
		}
	}
}

void KRunManager::OnAddMenuItems(KxMenu* menu)
{
	m_Menu = menu;
	KxIntVector list = m_RunMainOptions.GetAttributeVectorInt("MainEnabled");
	
	// If the list contains -1 as its first element, fill it with all indexes and save it back to settings
	if (!list.empty() && list.front() == -1)
	{
		list.clear();
		for (size_t i = 0; i < m_RunConfig->GetEntriesCount(KPRCE_TYPE_MAIN); i++)
		{
			list.emplace_back((int)i);
		}

		m_RunMainOptions.SetAttributeVectorInt("MainEnabled", list);
	}

	if (!list.empty())
	{
		for (int index: list)
		{
			const KRunManagerProgram* entry = m_RunConfig->GetEntryAt(KPRCE_TYPE_MAIN, index);
			if (entry)
			{
				KxMenuItem* item = menu->Add(new KxMenuItem(wxString::Format("%s %s", T("Generic.Run"), entry->GetName())));
				item->Enable(false);
				item->SetClientData((void*)entry);
				item->Bind(KxEVT_MENU_SELECT, [this, item, entry](KxMenuEvent& event)
				{
					OnRunEntry(item, *entry);
				});

				m_MenuItems.push_back(item);
			}
		}
	}
	else
	{
		menu->Add(new KxMenuItem(V("<$T(RunManager.NoPrograms)>")))->Enable(false);
	}

	m_Menu->Bind(KxEVT_MENU_OPEN, &KRunManager::OnMenuOpen, this);
}
void KRunManager::OnRunEntry(KxMenuItem* menuItem, const KRunManagerProgram& runEntry)
{
	KxStringVector missing = CheckPreMain();
	if (missing.empty())
	{
		if (CheckEntry(runEntry))
		{
			KxProgressDialog* dialog = BeginRunProcess();
			dialog->Show();

			RunPreMain(dialog);
			RunMain(dialog, runEntry);
			dialog->Close();
		}
	}
	else
	{
		KxTaskDialog dialog(wxTheApp->GetTopWindow(), KxID_NONE, T("RunManager.PreMainFileNotFound1"));
		dialog.SetMessage(T("RunManager.PreMainFileNotFound2"));
		dialog.SetExMessage(KxString::Join(missing, "\r\n"));
		dialog.SetMainIcon(KxICON_ERROR);
		dialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED, true);
		dialog.SetOptionEnabled(KxTD_SIZE_TO_CONTENT, true);
		dialog.ShowModal();
	}
}

void KRunManager::LoadProgramList()
{
	KxFileStream stream(GetProgramsListFile(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxXMLDocument xml(stream);

	m_ProgramList.clear();
	for (KxXMLNode node = xml.GetFirstChildElement("Programs").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		KRunManagerProgram& entry = m_ProgramList.emplace_back(KRunManagerProgram(node));
		if (!entry.IsOK())
		{
			m_ProgramList.pop_back();
		}
	}
}
void KRunManager::SaveProgramList() const
{
	KxXMLDocument xml;
	KxXMLNode rootNode = xml.NewElement("Programs");

	for (const KRunManagerProgram& entry: m_ProgramList)
	{
		KxXMLNode node = rootNode.NewElement("Entry");
		node.SetAttribute("RequiresVFS", entry.IsRequiresVFS());
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

bool KRunManager::RunEntry(const KRunManagerProgram& runEntry, KxProgressDialog* dialog)
{
	if (CheckEntry(runEntry))
	{
		DoRunEntry(runEntry, dialog);
		return true;
	}
	return false;
}
KxProcess* KRunManager::RunEntryDelayed(const KRunManagerProgram& runEntry, KxProgressDialog* dialog)
{
	if (CheckEntry(runEntry))
	{
		KxProcess* process = NULL;
		DoRunEntry(runEntry, dialog, &process);
		return process;
	}
	return NULL;
}
