#include "stdafx.h"
#include "KSaveManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "SaveManager/KSaveManager.h"
#include "KApp.h"
#include "KAux.h"

KSaveManagerConfig::KSaveManagerConfig(KGameInstance& instance, const KxXMLNode& node)
	:m_SaveInterface(node.GetAttribute("SaveFileFormat"))
{
	m_Location = node.GetFirstChildElement("Location").GetValue();

	// Load file filters
	for (KxXMLNode entryNode = node.GetFirstChildElement("FileFilters").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_FileFilters.emplace_back(KLabeledValue(entryNode.GetValue(), KVarExp(entryNode.GetAttribute("Label"))));
	}

	// Multi-file save config
	m_PrimarySaveExt = node.GetFirstChildElement("PrimaryExtension").GetValue();
	m_SecondarySaveExt = node.GetFirstChildElement("SecondaryExtension").GetValue();

	// Create manager
	m_Manager = new KSaveManager(node, this);
}
KSaveManagerConfig::~KSaveManagerConfig()
{
	delete m_Manager;
}

wxString KSaveManagerConfig::GetSaveInterface() const
{
	return KVarExp(m_SaveInterface);
}
wxString KSaveManagerConfig::GetLocation() const
{
	return KVarExp(m_Location);
}
