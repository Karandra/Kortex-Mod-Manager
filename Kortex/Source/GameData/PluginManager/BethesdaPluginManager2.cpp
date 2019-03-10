#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include "Bethesda2DisplayModel.h"
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxComparator.h>

namespace Kortex::PluginManager
{
	void BethesdaPluginManager2::OnInit()
	{
		BethesdaPluginManager::OnInit();
	}
	void BethesdaPluginManager2::OnExit()
	{
		BethesdaPluginManager::OnExit();
	}
	void BethesdaPluginManager2::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		BethesdaPluginManager::OnLoadInstance(instance, managerNode);
	}

	bool BethesdaPluginManager2::CheckExtension(const wxString& name) const
	{
		const wxString ext = name.AfterLast(wxS('.'));
		return KxComparator::IsEqual(ext, wxS("esp")) || KxComparator::IsEqual(ext, wxS("esm")) || KxComparator::IsEqual(ext, wxS("esl"));
	}
	
	void BethesdaPluginManager2::LoadNativeOrderBG()
	{
		ClearPlugins();
		FileTreeNode::CRefVector files = IModDispatcher::GetInstance()->Find(m_PluginsLocation, ModManager::DispatcherSearcher(wxEmptyString, KxFS_FILE), false);

		// Load names from 'Plugins.txt'. Activate those with '*' in front fo the file name.
		for (wxString& name: KxTextFile::ReadToArray(KVarExp(m_ActiveListFile)))
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
					// Remove asterix and check state
					bool isActive = false;
					if (name.StartsWith(wxS('*')))
					{
						isActive = true;
						name = name.AfterFirst(wxS('*'));
					}
					GetPlugins().emplace_back(CreatePlugin((*it)->GetFullPath(), isActive));
				}
			}
		}
	}
	void BethesdaPluginManager2::LoadNativeActiveBG()
	{
		// Empty for this implementation.
	}
	void BethesdaPluginManager2::SaveNativeOrderBG() const
	{
		// Base class version is fine. it will save both 'plugins.txt' and 'loadorder.txt',
		// but it will cal our 'OnWriteToActiveOrder' so these files will be written with correct content.
		BethesdaPluginManager::SaveNativeOrderBG();
	}

	wxString BethesdaPluginManager2::OnWriteToLoadOrder(const IGamePlugin& plugin) const
	{
		return plugin.GetName();
	}
	wxString BethesdaPluginManager2::OnWriteToActiveOrder(const IGamePlugin& plugin) const
	{
		return plugin.IsActive() ? wxS('*') + plugin.GetName() : plugin.GetName();
	}

	intptr_t BethesdaPluginManager2::CountLightActiveBefore(const IGamePlugin& plugin) const
	{
		intptr_t count = 0;
		for (const auto& currentPlugin: GetPlugins())
		{
			if (currentPlugin.get() == &plugin)
			{
				break;
			}

			const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
			if (currentPlugin->IsActive() && currentPlugin->QueryInterface(bethesdaPlugin) && bethesdaPlugin->IsLight())
			{
				count++;
			}
		}
		return count;
	}
	intptr_t BethesdaPluginManager2::OnGetPluginDisplayPriority(const IGamePlugin& plugin) const
	{
		const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
		if (plugin.QueryInterface(bethesdaPlugin) && bethesdaPlugin->IsLight())
		{
			return 0xFE;
		}
		return OnGetPluginPriority(plugin);
	}

	BethesdaPluginManager2::BethesdaPluginManager2()
	{
	}
	BethesdaPluginManager2::~BethesdaPluginManager2()
	{
	}

	std::unique_ptr<IGamePlugin> BethesdaPluginManager2::CreatePlugin(const wxString& fullPath, bool isActive)
	{
		auto plugin = std::make_unique<BethesdaPlugin2>(fullPath);
		plugin->SetActive(isActive);
		return plugin;
	}
	std::unique_ptr<IDisplayModel> BethesdaPluginManager2::CreateDisplayModel()
	{
		return std::make_unique<Bethesda2DisplayModel>(*this);
	}
}
