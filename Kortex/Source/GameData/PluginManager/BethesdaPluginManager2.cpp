#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
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
	void BethesdaPluginManager2::LoadNativeActiveBG()
	{
		// Load names from 'Plugins.txt' it they are not already added.
		// Activate all new added and existing items with same name.
		KxStringVector activeOrder = KxTextFile::ReadToArray(KVarExp(m_ActiveListFile));
		for (wxString& name: activeOrder)
		{
			if (!name.IsEmpty() && !name.StartsWith('#'))
			{
				// Remove asterix
				if (name.StartsWith('*'))
				{
					name = name.AfterFirst('*');
				}

				if (IGamePlugin* entry = FindPluginByName(name))
				{
					entry->SetActive(true);
				}
			}
		}
	}

	wxString BethesdaPluginManager2::OnWriteToLoadOrder(const IGamePlugin& plugin) const
	{
		return plugin.GetName();
	}
	wxString BethesdaPluginManager2::OnWriteToActiveOrder(const IGamePlugin& plugin) const
	{
		if (!plugin.IsStdContent())
		{
			return '*' + plugin.GetName();
		}
		return plugin.GetName();
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
	wxString BethesdaPluginManager2::OnFormatPriority(const IGamePlugin& plugin, intptr_t value) const
	{
		if (plugin.IsActive())
		{
			const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
			if (plugin.QueryInterface(bethesdaPlugin) && bethesdaPlugin->IsLight())
			{
				return wxString::Format("0x%02X:%03X", (int)value, (int)CountLightActiveBefore(plugin));
			}
			return IPluginManager::FormatPriority(plugin, value);
		}
		return wxEmptyString;
	}

	BethesdaPluginManager2::BethesdaPluginManager2()
	{
	}
	BethesdaPluginManager2::~BethesdaPluginManager2()
	{
	}

	std::unique_ptr<IGamePlugin> BethesdaPluginManager2::CreatePlugin(const wxString& fullPath, bool isActive) const
	{
		auto plugin = std::make_unique<BethesdaPlugin2>(fullPath);
		plugin->SetActive(isActive);
		return plugin;
	}
}
