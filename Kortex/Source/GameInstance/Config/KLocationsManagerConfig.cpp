#include "stdafx.h"
#include "KLocationsManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>

KLocationsManagerConfig::KLocationsManagerConfig(KGameInstance& profile, const KxXMLNode& node)
{
	// Set predefined locations
	m_Locations.emplace_back(KLabeledValue("$(AppSettings)", T("OpenLocation.AppSettings")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_ACTUAL_GAME_DIR), T("OpenLocation.GameRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_VIRTUAL_GAME_DIR), T("OpenLocation.VirtualGameRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_ACTUAL_CONFIG_DIR), T("OpenLocation.ConfigRootTarget")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_CONFIG_DIR), T("OpenLocation.VirtualConfigRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_OVERWRITES_DIR), T("OpenLocation.WriteTargetRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_INSTANCE_DIR), T("OpenLocation.CurrentProfileRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_INSTANCES_ROOT), T("OpenLocation.ProfilesRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_MODS_DIR), T("OpenLocation.ModsRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_SAVES_DIR), T("OpenLocation.Saves")));

	if (node.HasChildren())
	{
		// This will allow to insert a separator in locations menu
		m_Locations.emplace_back(KLabeledValue(wxEmptyString, wxEmptyString));

		// Load profile locations
		for (KxXMLNode entryNode = node.GetFirstChildElement("Entry"); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			KLabeledValue& value = m_Locations.emplace_back(KLabeledValue(V(entryNode.GetValue()), V(entryNode.GetAttribute("Label"))));
			if (value.GetValue().IsEmpty())
			{
				m_Locations.pop_back();
			}
		}
	}
}
KLocationsManagerConfig::~KLocationsManagerConfig()
{
}

KLabeledValueArray KLocationsManagerConfig::GetLocations() const
{
	KLabeledValueArray locations = m_Locations;
	for (KLabeledValue& value: locations)
	{
		value.SetLabel(V(value.GetLabelRaw()));
		value.SetValue(V(value.GetValue()));
	}
	return locations;
}
bool KLocationsManagerConfig::OpenLocation(const KLabeledValue& entry) const
{
	// Create the folder, shouldn't be harmful.
	KxFile tFolder(V(entry.GetValue()));
	tFolder.CreateFolder();

	return KxShell::Execute(KApp::Get().GetTopWindow(), tFolder.GetFullPath(), "open");
}
