#include "stdafx.h"
#include "KLocationsManagerConfig.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "Utility/KImageProvider.h"
#include "Utility/KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxMenu.h>

namespace Kortex
{
	void KLocationsManagerConfig::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		// Set predefined locations
		using namespace Variables;

		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_APP_SETTINGS_DIR), KTr("OpenLocation.AppSettings")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_ACTUAL_GAME_DIR), KTr("OpenLocation.GameRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_VIRTUAL_GAME_DIR), KTr("OpenLocation.VirtualGameRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_ACTUAL_CONFIG_DIR), KTr("OpenLocation.ConfigRootTarget")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_CONFIG_DIR), KTr("OpenLocation.VirtualConfigRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_OVERWRITES_DIR), KTr("OpenLocation.WriteTargetRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_INSTANCE_DIR), KTr("OpenLocation.CurrentProfileRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_INSTANCES_DIR), KTr("OpenLocation.ProfilesRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_MODS_DIR), KTr("OpenLocation.ModsRoot")));
		m_Locations.emplace_back(KLabeledValue(WrapAsInline(KVAR_SAVES_DIR), KTr("OpenLocation.Saves")));

		if (node.HasChildren())
		{
			// This will allow to insert a separator in locations menu
			m_Locations.emplace_back(KLabeledValue(wxEmptyString, wxEmptyString));

			// Load profile locations
			for (KxXMLNode entryNode = node.GetFirstChildElement("Entry"); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
			{
				KLabeledValue& value = m_Locations.emplace_back(KLabeledValue(KVarExp(entryNode.GetValue()), KVarExp(entryNode.GetAttribute("Label"))));
				if (value.GetValue().IsEmpty())
				{
					m_Locations.pop_back();
				}
			}
		}
	}

	KLabeledValue::Vector KLocationsManagerConfig::GetLocations() const
	{
		return KAux::ExpandVariablesInVector(m_Locations);
	}
	bool KLocationsManagerConfig::OpenLocation(const KLabeledValue& entry) const
	{
		// Create the folder, shouldn't be harmful.
		KxFile tFolder(KVarExp(entry.GetValue()));
		tFolder.CreateFolder();

		return KxShell::Execute(IApplication::GetInstance()->GetTopWindow(), tFolder.GetFullPath(), "open");
	}

	void KLocationsManagerConfig::OnAddMainMenuItems(KxMenu& mainMenu)
	{
		KxMenu* locationsMenu = new KxMenu();
		for (const KLabeledValue& entry: m_Locations)
		{
			if (!entry.HasLabel() && !entry.HasLabel())
			{
				locationsMenu->AddSeparator();
			}
			else
			{
				KxMenuItem* item = locationsMenu->Add(new KxMenuItem(KVarExp(entry.GetLabel())));
				item->SetBitmap(KGetBitmap(KIMG_FOLDER));
				item->Bind(KxEVT_MENU_SELECT, [this, &entry](KxMenuEvent& event)
				{
					OpenLocation(KLabeledValue(KVarExp(entry.GetValue())));
				});
			}
		}
		mainMenu.Add(locationsMenu, KTr("MainMenu.OpenLocation"))->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
		mainMenu.AddSeparator();
	}
}
