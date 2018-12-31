#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "UI/KWorkspace.h"
#include <Kortex/Events.hpp>
#include "Utility/KUPtrVectorUtil.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxComparator.h>

namespace Kortex
{
	namespace PluginManager::Internal
	{
		const SimpleManagerInfo TypeInfo("PluginManager", "PluginManager.Name");
	}

	intptr_t IPluginManager::OnGetOrderIndex(const IGamePlugin& plugin) const
	{
		const IGamePlugin::Vector& items = GetPlugins();
		auto it = std::find_if(items.begin(), items.end(), [&plugin](const auto& v)
		{
			return v.get() == &plugin;
		});
		if (it != items.end())
		{
			return std::distance(items.begin(), it);
		}
		return -1;
	}
	void IPluginManager::OnVirtualTreeInvalidated(IEvent& event)
	{
		Invalidate();
	}

	IPluginManager::IPluginManager()
		:ManagerWithTypeInfo(GameDataModule::GetInstance())
	{
		IEvent::Bind(Events::ProfileSelected, &IPluginManager::OnVirtualTreeInvalidated, this);
		IEvent::Bind(Events::ModsReordered, &IPluginManager::OnVirtualTreeInvalidated, this);
		IEvent::Bind(Events::ModToggled, &IPluginManager::OnVirtualTreeInvalidated, this);
		IEvent::Bind(Events::ModFilesChanged, &IPluginManager::OnVirtualTreeInvalidated, this);
	}

	bool IPluginManager::IsPluginActive(const wxString& pluginName) const
	{
		const IGamePlugin* entry = FindPluginByName(pluginName);
		return entry && entry->IsActive();
	}
	void IPluginManager::SetAllPluginsActive(bool isActive)
	{
		for (auto& plugin: GetPlugins())
		{
			plugin->SetActive(isActive);
		}
	}
}
