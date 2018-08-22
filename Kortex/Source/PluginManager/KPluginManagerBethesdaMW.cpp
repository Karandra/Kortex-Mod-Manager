#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KPluginManagerBethesdaMorrowind.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "Profile/KPluginManagerConfig.h"
#include "GameConfig/KGameConfigWorkspace.h"
#include "UI/KWorkspace.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxINI.h>
#include <KxFramework/KxFile.h>

void KPluginManagerBethesdaMorrowind::ReadOrderMW(const KxINI& ini)
{
	auto files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);

	// Load all names from 'Game Files Order' section. Discard non-existing files.
	KxStringVector loadOrder = ini.GetKeyNames("Game Files Order");
	for (const wxString& name: loadOrder)
	{
		// Check whether plugin with this name exist
		wxString nameL = KxString::ToLower(name);
		auto it = std::find_if(files.begin(), files.end(), [&nameL](const KxFileFinderItem& item)
		{
			return KxString::ToLower(item.GetName()) == nameL;
		});

		if (it != files.end())
		{
			KPMPluginEntryType type = GetPluginTypeFromPath(nameL);
			if (type != KPMPE_TYPE_INVALID)
			{
				KPMPluginEntry* entry = GetEntries().emplace_back(NewPluginEntry(name, false, type)).get();
				entry->SetFullPath(it->GetFullPath());
				SetParentModAndPluginFile(entry);
			}
		}
	}

	// Load files form 'Data Files' folder. Don't add already existing
	for (const KxFileFinderItem& item: files)
	{
		KPMPluginEntryType type = GetPluginTypeFromPath(item.GetName());
		if (type != KPMPE_TYPE_INVALID && !FindPluginByName(item.GetName()))
		{
			KPMPluginEntry* entry = GetEntries().emplace_back(NewPluginEntry(item.GetName(), false, type)).get();
			entry->SetFullPath(item.GetFullPath());
			SetParentModAndPluginFile(entry);
		}
	}
}
void KPluginManagerBethesdaMorrowind::ReadActiveMW(const KxINI& ini)
{
	KxStringVector activeOrder = ini.GetKeyNames("Game Files");
	for (const wxString& nameID: activeOrder)
	{
		wxString name = ini.GetValue("Game Files", nameID);
		KPMPluginEntry* entry = FindPluginByName(name);
		if (entry)
		{
			entry->SetEnabled(true);
		}
	}
}
void KPluginManagerBethesdaMorrowind::WriteOrderMW(KxINI& ini) const
{
	wxDateTime fileTime = wxDateTime::Now() - wxTimeSpan(0, GetEntries().size());
	const wxTimeSpan timeStep(0, 1);

	int i = 0;
	ini.RemoveSection("Game Files Order");
	for (const KModListPluginEntry& tListItem: KModManager::GetListManager().GetCurrentList().GetPlugins())
	{
		if (const KPMPluginEntry* entry = tListItem.GetPluginEntry())
		{
			ini.SetValue("Game Files Order", wxString::Format("GameFile%d", i), OnWriteToLoadOrder(entry));
			i++;

			if (ShouldChangeFileModificationDate())
			{
				KxFile(entry->GetFullPath()).SetFileTime(fileTime, KxFILETIME_MODIFICATION);
				fileTime.Add(timeStep);
			}
		}
	}
}
void KPluginManagerBethesdaMorrowind::WriteActiveMW(KxINI& ini) const
{
	int i = 0;
	ini.RemoveSection("Game Files");
	for (auto& entry: GetEntries())
	{
		if (entry->IsEnabled())
		{
			ini.SetValue("Game Files", wxString::Format("GameFile%d", i), OnWriteToActiveOrder(entry.get()));
			i++;
		}
	}
}

void KPluginManagerBethesdaMorrowind::LoadNativeOrderBG()
{
	Clear();

	KxFileStream file(KModManager::GetDispatcher().GetTargetPath(m_PluginsListFile), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxINI ini(file);
	if (ini.IsOK())
	{
		ReadOrderMW(ini);
		ReadActiveMW(ini);

		if (ShouldSortByFileModificationDate())
		{
			SortByDate();
		}
	}
}
void KPluginManagerBethesdaMorrowind::LoadNativeActiveBG()
{
}
void KPluginManagerBethesdaMorrowind::SaveNativeOrderBG() const
{
	KxFileStream file(m_PluginsListFile, KxFS_ACCESS_RW, KxFS_DISP_OPEN_ALWAYS, KxFS_SHARE_READ);
	KxINI ini(file);
	if (ini.IsOK())
	{
		WriteActiveMW(ini);
		WriteOrderMW(ini);
		ini.Save(file);
		file.Close();

		if (KGameConfigWorkspace* workspace = KGameConfigWorkspace::GetInstance())
		{
			workspace->ScheduleRefresh();
		}
	}
}

KPluginManagerBethesdaMorrowind::KPluginManagerBethesdaMorrowind(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig)
	:KPluginManagerBethesdaGeneric(interfaceName, configNode, profilePluginConfig)
{
	m_PluginsLocation = "Data Files";
	m_PluginsListFile = "Morrowind.ini";
}
KPluginManagerBethesdaMorrowind::~KPluginManagerBethesdaMorrowind()
{
}

bool KPluginManagerBethesdaMorrowind::Save()
{
	return KPluginManagerBethesdaGeneric::Save();
}
bool KPluginManagerBethesdaMorrowind::Load()
{
	return KPluginManagerBethesdaGeneric::Load();
}
bool KPluginManagerBethesdaMorrowind::LoadNativeOrder()
{
	return KPluginManagerBethesdaGeneric::LoadNativeOrder();
}
