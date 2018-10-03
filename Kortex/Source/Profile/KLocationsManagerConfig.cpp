#include "stdafx.h"
#include "KLocationsManagerConfig.h"
#include "KProfile.h"
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>

KLocationsManagerConfig::KLocationsManagerConfig(KProfile& profile, const KxXMLNode& node)
{
	// Set predefined locations
	m_Locations.emplace_back(KLabeledValue("$(AppSettings)", T("OpenLocation.AppSettings")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_GAME_ROOT), T("OpenLocation.GameRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_VIRTUAL_GAME_ROOT), T("OpenLocation.VirtualGameRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_CONFIG_ROOT_TARGET), T("OpenLocation.ConfigRootTarget")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_CONFIG_ROOT_LOCAL), T("OpenLocation.VirtualConfigRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_OVERWRITES_ROOT), T("OpenLocation.WriteTargetRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_PROFILE_ROOT), T("OpenLocation.CurrentProfileRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_PROFILES_ROOT), T("OpenLocation.ProfilesRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_MODS_ROOT), T("OpenLocation.ModsRoot")));
	m_Locations.emplace_back(KLabeledValue(KVAR(KVAR_SAVES_ROOT_LOCAL), T("OpenLocation.Saves")));

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
