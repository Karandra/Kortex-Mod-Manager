#include "stdafx.h"
#include "IProgramManager.h"
#include <Kortex/Common/Programs.hpp>
#include <Kortex/Events.hpp>
#include "KBitmapSize.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileItem.h>
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxString.h>

namespace
{
	using namespace Kortex;

	void InitProcessPaths(KxProcess& process, const wxString& executable, const wxString& arguments = {}, const wxString& workingDirectory = {})
	{
		process.SetExecutablePath(executable);
		process.SetWorkingFolder(workingDirectory.IsEmpty() ? executable.BeforeLast(wxS('\\')) : workingDirectory);
		process.SetArguments(arguments);
	}
	void InitProcessPaths(KxProcess& process, const IProgramEntry& runEntry)
	{
		InitProcessPaths(process, runEntry.GetExecutable(), runEntry.GetArguments(), runEntry.GetWorkingDirectory());
	}
}

namespace Kortex
{
	namespace ProgramManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ProgramManager", "ProgramManager.Name");
	}

	void IProgramManager::OnAddMainMenuItems(KxMenu& menu)
	{
		for (auto& entry: GetProgramList())
		{
			if (entry->ShouldShowInMainMenu())
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(KxString::Format("%1 %2", KTr("Generic.Run"), entry->GetName())));
				item->Enable(entry->CanRunNow());
				item->Bind(KxEVT_MENU_SELECT, [this, &entry](KxMenuEvent& event)
				{
					RunEntry(*entry);
				});

				if (!CheckProgramIcons(*entry))
				{
					LoadProgramIcons(*entry);
				}
				item->SetBitmap(entry->GetSmallBitmap().GetBitmap());
			}
		}
	}

	std::unique_ptr<KxProcess> IProgramManager::DoCreateProcess(const IProgramEntry& entry) const
	{
		auto process = std::make_unique<KxProcess>();
		InitProcessPaths(*process, entry);
		
		// Set options
		process->SetOptionEnabled(KxPROCESS_WAIT_END, false);
		process->SetOptionEnabled(KxPROCESS_WAIT_INPUT_IDLE, false);
		process->SetOptionEnabled(KxPROCESS_DETACHED, false);

		// Save 'IProgramEntry' pointer
		process->SetClientData(const_cast<IProgramEntry*>(&entry));

		return process;
	}
	int IProgramManager::DoRunProcess(std::unique_ptr<KxProcess> process) const
	{
		IProgramEntry* entry = static_cast<IProgramEntry*>(process->GetClientData());
		entry->OnRun();

		return process->Run(KxPROCESS_RUN_SYNC);
	}
	bool IProgramManager::DoCheckEntry(const IProgramEntry& entry) const
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

	void IProgramManager::LoadProgramsFromXML(IProgramEntry::Vector& programs, const KxXMLNode& rootNode)
	{
		for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			auto program = NewProgram();
			program->Load(node);
			if (program->IsOK())
			{
				programs.emplace_back(std::move(program));
			}
		}
	}

	IProgramManager::IProgramManager()
		:ManagerWithTypeInfo(KProgramModule::GetInstance())
	{
	}

	std::unique_ptr<KxProcess> IProgramManager::CreateProcess(const IProgramEntry& entry) const
	{
		return DoCreateProcess(entry);
	}
	int IProgramManager::RunProcess(std::unique_ptr<KxProcess> process) const
	{
		return DoRunProcess(std::move(process));
	}
	int IProgramManager::RunEntry(const IProgramEntry& entry) const
	{
		if (DoCheckEntry(entry))
		{
			return DoRunProcess(DoCreateProcess(entry));
		}
		return -1;
	}

	wxBitmap IProgramManager::LoadProgramIcon(const IProgramEntry& entry, BitmapVariant bitmapVariant) const
	{
		wxBitmap bitmap;
		wxString iconPath = entry.GetIconPath();

		// If icon points at 'exe' ask system for its icon, otherwise load ourselves.
		if (KAux::IsSingleFileExtensionMatches(iconPath, wxS("exe")))
		{
			bitmap = KxShell::GetFileIcon(iconPath, bitmapVariant == BitmapVariant::Small);
		}
		else
		{
			bitmap.LoadFile(iconPath, wxBITMAP_TYPE_ANY);
		}

		// If image wasn't loaded, use default icon for 'exe' files.
		if (!bitmap.IsOk())
		{
			KxFileItem item;
			item.SetName(wxS(".exe"));
			item.SetNormalAttributes();
			bitmap = KxShell::GetFileIcon(item, bitmapVariant == BitmapVariant::Small);
		}

		// If bitmap is loaded it should be in required size (by 'bitmapVariant'), but if not, correct that.
		KBitmapSize bitmapSize;
		if (bitmapVariant == BitmapVariant::Small)
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
		return bitmap;
	}
	void IProgramManager::LoadProgramIcons(IProgramEntry& entry) const
	{
		entry.GetSmallBitmap().SetBitmap(LoadProgramIcon(entry, BitmapVariant::Small));
		entry.GetLargeBitmap().SetBitmap(LoadProgramIcon(entry, BitmapVariant::Large));
	}
	bool IProgramManager::CheckProgramIcons(const IProgramEntry& entry) const
	{
		return entry.GetSmallBitmap().HasBitmap() && entry.GetLargeBitmap().HasBitmap();
	}
}
