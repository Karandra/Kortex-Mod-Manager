#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex::PluginManager
{
	void BethesdaPluginManager::OnInit()
	{
		BasePluginManager::OnInit();
	}
	void BethesdaPluginManager::OnExit()
	{
		BasePluginManager::OnExit();
	}
	void BethesdaPluginManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		BasePluginManager::OnLoadInstance(instance, managerNode);

		// Load LOOT API config
		KxXMLNode lootAPINode = managerNode.GetFirstChildElement("LibLoot");
		m_LibLootConfig.OnLoadInstance(instance, lootAPINode);
		if (m_LibLootConfig.IsOK())
		{
			m_LootAPI = std::make_unique<LibLoot>();
		}

		m_ActiveListFile = managerNode.GetFirstChildElement("ActiveList").GetValue();
		m_OrderListFile = managerNode.GetFirstChildElement("OrderList").GetValue();

		// Don't expand them here. These strings may contain date-time variables
		m_ActiveFileHeader = managerNode.GetFirstChildElement("ActiveListHeader").GetValue();
		m_OrderFileHeader = managerNode.GetFirstChildElement("OrderListHeader").GetValue();

		m_ShouldChangeFileModificationDate = managerNode.GetFirstChildElement("ChangeFileModificationDate").GetValueBool();
		m_ShouldSortByFileModificationDate = managerNode.GetFirstChildElement("SortByFileModificationDate").GetValueBool();
	}

	void BethesdaPluginManager::SortByDate()
	{
		IGamePlugin::Vector& plugins = GetPlugins();
		std::sort(plugins.begin(), plugins.end(), [this](const auto& entry1, const auto& entry2)
		{
			wxFileName file1(entry1->GetFullPath());
			wxFileName file2(entry2->GetFullPath());

			return file1.GetModificationTime() < file2.GetModificationTime();
		});
	}
	bool BethesdaPluginManager::CheckExtension(const wxString& name) const
	{
		const wxString ext = name.AfterLast(wxS('.'));
		return KxComparator::IsEqual(ext, wxS("esp")) || KxComparator::IsEqual(ext, wxS("esm"));
	}

	IGamePlugin::RefVector BethesdaPluginManager::CollectDependentPlugins(const IGamePlugin& plugin, bool firstOnly) const
	{
		IGamePlugin::RefVector dependentList;
		dependentList.reserve(firstOnly ? 1 : 0);

		for (auto& currentPlugin: GetPlugins())
		{
			if (currentPlugin->IsActive())
			{
				const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
				if (currentPlugin->QueryInterface(bethesdaPlugin))
				{
					KxStringVector dependenciesList = bethesdaPlugin->GetRequiredPlugins();
					auto it = std::find_if(dependenciesList.begin(), dependenciesList.end(), [&plugin](const wxString& depName)
					{
						return KxComparator::IsEqual(plugin.GetName(), depName);
					});
					if (it != dependenciesList.end())
					{
						dependentList.push_back(currentPlugin.get());
						if (firstOnly)
						{
							break;
						}
					}
				}
			}
		}

		return dependentList;
	}
	bool BethesdaPluginManager::HasDependentPlugins(const IGamePlugin& plugin) const
	{
		return !CollectDependentPlugins(plugin, true).empty();
	}
	IGamePlugin::RefVector BethesdaPluginManager::GetDependentPlugins(const IGamePlugin& plugin) const
	{
		return CollectDependentPlugins(plugin, false);
	}

	wxString BethesdaPluginManager::OnWriteToLoadOrder(const IGamePlugin& plugin) const
	{
		return plugin.GetName();
	}
	wxString BethesdaPluginManager::OnWriteToActiveOrder(const IGamePlugin& plugin) const
	{
		return plugin.IsActive() ? plugin.GetName() : wxEmptyString;
	}
	void BethesdaPluginManager::CreateWorkspace()
	{
		new Workspace();
	}

	void BethesdaPluginManager::LoadNativeOrderBG()
	{
		ClearPlugins();
		FileTreeNode::CRefVector files = IModDispatcher::GetInstance()->Find(m_PluginsLocation, [](const FileTreeNode& node)
		{
			return node.IsFile();
		}, false);

		// Load from 'LoadOrder.txt'
		for (const wxString& name: KxTextFile::ReadToArray(KVarExp(m_OrderListFile)))
		{
			// Find whether plugin with this name exist
			auto it = std::find_if(files.begin(), files.end(), [&name](const FileTreeNode* node)
			{
				return KxComparator::IsEqual(node->GetName(), name);
			});

			if (!name.StartsWith('#') && it != files.end())
			{
				if (CheckExtension(name))
				{
					GetPlugins().emplace_back(CreatePlugin((*it)->GetFullPath(), false));
				}
			}
		}

		// Load new plugins from folder
		for (const FileTreeNode* fileNode: files)
		{
			if (CheckExtension(fileNode->GetName()) && !FindPluginByName(fileNode->GetName()))
			{
				GetPlugins().emplace_back(CreatePlugin(fileNode->GetFullPath(), false));
			}
		}

		if (ShouldSortByFileModificationDate())
		{
			SortByDate();
		}
		LoadNativeActiveBG();

		// Enable all std-content
		for (auto& entry: GetPlugins())
		{
			if (entry->GetStdContentEntry())
			{
				entry->SetActive(true);
			}
		}
	}
	void BethesdaPluginManager::LoadNativeActiveBG()
	{
		// Load names from 'Plugins.txt' it they are not already added.
		// Activate all new added and existing items with same name.
		for (const wxString& name: KxTextFile::ReadToArray(KVarExp(m_ActiveListFile)))
		{
			if (IGamePlugin* plugin = FindPluginByName(name))
			{
				plugin->SetActive(true);
			}
		}
	}
	void BethesdaPluginManager::SaveNativeOrderBG() const
	{
		const bool modFileDate = ShouldChangeFileModificationDate();

		// Initialize starting time point to (current time - entries count) minutes,
		// so incrementing it by one minute gives no "overflows" into future.
		wxDateTime fileTime = wxDateTime::Now() - wxTimeSpan(0, GetPlugins().size());
		const wxTimeSpan timeStep(0, 1);

		// Lists
		KxStringVector loadOrder;
		loadOrder.emplace_back(KVarExp(m_OrderFileHeader));

		KxStringVector activeOrder;
		activeOrder.emplace_back(KVarExp(m_ActiveFileHeader));

		// Write order
		for (const GameInstance::ProfilePlugin& listItem: IGameInstance::GetActive()->GetActiveProfile()->GetPlugins())
		{
			if (const IGamePlugin* plugin = listItem.GetPlugin())
			{
				if (loadOrder.emplace_back(OnWriteToLoadOrder(*plugin)).IsEmpty())
				{
					loadOrder.pop_back();
				}
				if (activeOrder.emplace_back(OnWriteToActiveOrder(*plugin)).IsEmpty())
				{
					activeOrder.pop_back();
				}

				if (modFileDate)
				{
					KxFile(plugin->GetFullPath()).SetFileTime(fileTime, KxFILETIME_MODIFICATION);
					fileTime.Add(timeStep);
				}
			}
		}

		// Save files
		const wxString orderListFile = KVarExp(m_OrderListFile);
		KxFile(orderListFile.BeforeLast('\\')).CreateFolder();
		KxTextFile::WriteToFile(orderListFile, loadOrder, wxTextFileType_Dos);

		const wxString activeListFile = KVarExp(m_ActiveListFile);
		KxFile(activeListFile.BeforeLast('\\')).CreateFolder();
		KxTextFile::WriteToFile(activeListFile, activeOrder, wxTextFileType_Dos);
	}

	BethesdaPluginManager::BethesdaPluginManager()
		:m_PluginsLocation("Data")
	{
	}
	BethesdaPluginManager::~BethesdaPluginManager()
	{
	}

	std::unique_ptr<IGamePlugin> BethesdaPluginManager::CreatePlugin(const wxString& fullPath, bool isActive)
	{
		auto plugin = std::make_unique<BethesdaPlugin>(fullPath);
		plugin->SetActive(isActive);
		return plugin;
	}
	std::unique_ptr<IPluginReader> BethesdaPluginManager::CreatePluginReader()
	{
		using namespace PluginManager;
		using namespace PluginManager::Internal;

		const wxString& name = m_Config.GetPluginImplementation();
		if (name == PluginImplementation::BethesdaMorrowind)
		{
			return std::make_unique<PluginManager::BethesdaPluginReaderMorrowind>();
		}
		else if (name == PluginImplementation::BethesdaOblivion)
		{
			return std::make_unique<BethesdaPluginReaderOblivion>();
		}
		else if (name == PluginImplementation::BethesdaSkyrim)
		{
			return std::make_unique<BethesdaPluginReaderSkyrim>();
		}
		return nullptr;
	}

	IWorkspace::RefVector BethesdaPluginManager::EnumWorkspaces() const
	{
		return ToWorkspacesList(Workspace::GetInstance());
	}
	std::unique_ptr<IDisplayModel> BethesdaPluginManager::CreateDisplayModel()
	{
		return std::make_unique<BethesdaDisplayModel>();
	}

	wxString BethesdaPluginManager::GetPluginTypeName(const IGamePlugin& plugin) const
	{
		const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
		if (plugin.QueryInterface(bethesdaPlugin))
		{
			return GetPluginTypeName(bethesdaPlugin->IsMaster(), bethesdaPlugin->IsLight());
		}
		return GetPluginTypeName(false, false);
	}
	wxString BethesdaPluginManager::GetPluginTypeName(bool isMaster, bool isLight) const
	{
		if (isMaster && isLight)
		{
			return KVarExp(wxS("$T(PluginManager.PluginType.Master) ($T(PluginManager.PluginType.Light))"));
		}
		if (isMaster)
		{
			return KTr(wxS("PluginManager.PluginType.Master"));
		}
		if (isLight)
		{
			return KTr(wxS("PluginManager.PluginType.Light"));
		}
		return KTr(wxS("PluginManager.PluginType.Normal"));
	}
	const IGameMod* BethesdaPluginManager::FindOwningMod(const IGamePlugin& plugin) const
	{
		const FileTreeNode* node = IModDispatcher::GetInstance()->ResolveLocation(GetPluginRootRelativePath(plugin.GetName()));
		if (node)
		{
			return &node->GetMod();
		}
		return nullptr;
	}

	void BethesdaPluginManager::Save() const
	{
		SaveNativeOrderBG();
	}
	void BethesdaPluginManager::Load()
	{
		ClearPlugins();

		if (IGameProfile* profile = IGameInstance::GetActiveProfile())
		{
			FileTreeNode::CRefVector files = IModDispatcher::GetInstance()->Find(m_PluginsLocation, [](const FileTreeNode& node)
			{
				return node.IsFile();
			}, false);

			for (const GameInstance::ProfilePlugin& profilePlugin: profile->GetPlugins())
			{
				// Find whether plugin with this name exist
				auto it = std::find_if(files.begin(), files.end(), [&profilePlugin](const FileTreeNode* item)
				{
					return KxComparator::IsEqual(item->GetName(), profilePlugin.GetName());
				});

				if (it != files.end())
				{
					if (CheckExtension(profilePlugin.GetName()))
					{
						GetPlugins().emplace_back(CreatePlugin((*it)->GetFullPath(), false));
					}
				}
			}

			// Load files form 'Data' folder. Don't add already existing
			for (const FileTreeNode* fileNode: files)
			{
				if (CheckExtension(fileNode->GetName()))
				{
					if (FindPluginByName(fileNode->GetName()) == nullptr)
					{
						GetPlugins().emplace_back(CreatePlugin(fileNode->GetFullPath(), false));
					}
				}
			}

			// Check active
			for (const GameInstance::ProfilePlugin& profilePlugin: profile->GetPlugins())
			{
				IGamePlugin* entry = FindPluginByName(profilePlugin.GetName());
				if (entry)
				{
					entry->SetActive(profilePlugin.IsActive());
				}
			}

			// Sort by file modification date if needed otherwise all elements already in correct order
			if (ShouldSortByFileModificationDate())
			{
				SortByDate();
			}
		}
	}
	void BethesdaPluginManager::LoadNativeOrder()
	{
		LoadNativeOrderBG();
		Save();
	}
}

namespace Kortex::PluginManager
{
	void LootAPIConfig::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		#if _WIN64
		m_Librray.Load(IApplication::GetInstance()->GetDataFolder() + "\\PluginManager\\LibLoot x64\\loot.dll");
		#else
		m_Librray.Load(IApplication::GetInstance()->GetDataFolder() + "\\PluginManager\\LibLoot x86\\loot.dll");
		#endif

		m_Branch = node.GetFirstChildElement("Branch").GetValue();
		m_Repository = node.GetFirstChildElement("Repository").GetValue();
		m_FolderName = node.GetFirstChildElement("FolderName").GetValue();
		m_LocalGamePath = node.GetFirstChildElement("LocalGamePath").GetValue();
	}

	bool LootAPIConfig::IsOK() const
	{
		return m_Librray.IsOK() && !m_Branch.IsEmpty() && !m_Repository.IsEmpty() && !m_FolderName.IsEmpty() && !m_LocalGamePath.IsEmpty();
	}

	wxString LootAPIConfig::GetBranch() const
	{
		return KVarExp(m_Branch);
	}
	wxString LootAPIConfig::GetRepository() const
	{
		return KVarExp(m_Repository);
	}
	wxString LootAPIConfig::GetFolderName() const
	{
		return KVarExp(m_FolderName);
	}
	wxString LootAPIConfig::GetLocalGamePath() const
	{
		return KVarExp(m_LocalGamePath);
	}
}
