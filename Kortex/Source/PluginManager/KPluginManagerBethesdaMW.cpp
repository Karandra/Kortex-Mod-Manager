#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KPluginManagerBethesdaMW.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "Profile/KPluginManagerConfig.h"
#include "GameConfig/KGameConfigWorkspace.h"
#include "UI/KWorkspace.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxINI.h>
#include <KxFramework/KxFile.h>

void KPluginManagerBethesdaMW::ReadOrderMW(const KxINI& ini)
{
	auto files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);

	// Load all names from 'Game Files Order' section. Discard non-existing files.
	KxStringVector loadOrder = ini.GetKeyNames("Game Files Order");
	for (const wxString& name: loadOrder)
	{
		// Check whether plugin with this name exist
		wxString nameL = KxString::ToLower(name);
		auto it = std::find_if(files.begin(), files.end(), [&nameL](const KFileTreeNode* node)
		{
			return KxString::ToLower(node->GetName()) == nameL;
		});

		if (it != files.end())
		{
			if (CheckExtension(nameL))
			{
				auto& entry = GetEntries().emplace_back(NewPluginEntry(name, false));
				entry->SetFullPath((*it)->GetFullPath());
				entry->SetParentMod(FindParentMod(*entry));
			}
		}
	}

	// Load files form 'Data Files' folder. Don't add already existing
	for (const KFileTreeNode* fileNode: files)
	{
		if (CheckExtension(fileNode->GetName()))
		{
			auto& entry = GetEntries().emplace_back(NewPluginEntry(fileNode->GetName(), false));
			entry->SetFullPath(fileNode->GetFullPath());
			entry->SetParentMod(FindParentMod(*entry));
		}
	}
}
void KPluginManagerBethesdaMW::ReadActiveMW(const KxINI& ini)
{
	KxStringVector activeOrder = ini.GetKeyNames("Game Files");
	for (const wxString& nameID: activeOrder)
	{
		wxString name = ini.GetValue("Game Files", nameID);
		KPluginEntry* entry = FindPluginByName(name);
		if (entry)
		{
			entry->SetEnabled(true);
		}
	}
}
void KPluginManagerBethesdaMW::WriteOrderMW(KxINI& ini) const
{
	wxDateTime fileTime = wxDateTime::Now() - wxTimeSpan(0, GetEntries().size());
	const wxTimeSpan timeStep(0, 1); // One minute

	int i = 0;
	ini.RemoveSection("Game Files Order");
	for (const KModListPlugin& listItem: KModManager::GetListManager().GetCurrentList().GetPlugins())
	{
		if (const KPluginEntry* entry = listItem.GetPluginEntry())
		{
			ini.SetValue("Game Files Order", wxString::Format("GameFile%d", i), OnWriteToLoadOrder(*entry));
			i++;

			if (ShouldChangeFileModificationDate())
			{
				KxFile(entry->GetFullPath()).SetFileTime(fileTime, KxFILETIME_MODIFICATION);
				fileTime.Add(timeStep);
			}
		}
	}
}
void KPluginManagerBethesdaMW::WriteActiveMW(KxINI& ini) const
{
	int i = 0;
	ini.RemoveSection("Game Files");
	for (auto& entry: GetEntries())
	{
		if (entry->IsEnabled())
		{
			ini.SetValue("Game Files", wxString::Format("GameFile%d", i), OnWriteToActiveOrder(*entry));
			i++;
		}
	}
}

void KPluginManagerBethesdaMW::LoadNativeOrderBG()
{
	Clear();

	KxFileStream stream(KModManager::GetDispatcher().ResolveLocationPath(m_PluginsListFile), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxINI ini(stream);
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
void KPluginManagerBethesdaMW::LoadNativeActiveBG()
{
}
void KPluginManagerBethesdaMW::SaveNativeOrderBG() const
{
	KxFileStream stream(m_PluginsListFile, KxFS_ACCESS_RW, KxFS_DISP_OPEN_ALWAYS, KxFS_SHARE_READ);
	KxINI ini(stream);
	if (ini.IsOK())
	{
		WriteActiveMW(ini);
		WriteOrderMW(ini);
		ini.Save(stream);
		stream.Close();

		if (KGameConfigWorkspace* workspace = KGameConfigWorkspace::GetInstance())
		{
			workspace->ScheduleReload();
		}
	}
}

KPluginManagerBethesdaMW::KPluginManagerBethesdaMW(const wxString& interfaceName, const KxXMLNode& configNode)
	:KPluginManagerBethesda(interfaceName, configNode)
{
	m_PluginsLocation = "Data Files";
	m_PluginsListFile = "Morrowind.ini";
}
KPluginManagerBethesdaMW::~KPluginManagerBethesdaMW()
{
}

void KPluginManagerBethesdaMW::Save() const
{
	KPluginManagerBethesda::Save();
}
void KPluginManagerBethesdaMW::Load()
{
	KPluginManagerBethesda::Load();
}
void KPluginManagerBethesdaMW::LoadNativeOrder()
{
	KPluginManagerBethesda::LoadNativeOrder();
}
