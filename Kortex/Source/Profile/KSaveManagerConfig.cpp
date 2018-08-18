#include "stdafx.h"
#include "KSaveManagerConfig.h"
#include "KProfile.h"
#include "SaveManager/KSaveManager.h"
#include "SaveManager/KSMSaveFile.h"
#include "SaveManager/KSMSaveFileBethesdaMorrowind.h"
#include "SaveManager/KSMSaveFileBethesdaOblivion.h"
#include "SaveManager/KSMSaveFileBethesdaSkyrim.h"
#include "SaveManager/KSMSaveFileBethesdaSkyrimSE.h"
#include "SaveManager/KSMSaveFileBethesdaFallout3.h"
#include "SaveManager/KSMSaveFileBethesdaFalloutNV.h"
#include "SaveManager/KSMSaveFileBethesdaFallout4.h"
#include "KApp.h"
#include "KAux.h"

KxSingletonPtr_Define(KSaveManagerConfig);

KSaveManagerConfig::KSaveManagerConfig(KProfile& profile, KxXMLNode& node)
	:m_SaveFileFormat(node.GetAttribute("SaveFileFormat"))
{
	m_SavesFolder = V(node.GetFirstChildElement("SavesFolder").GetValue());

	// Load file filters
	for (KxXMLNode entryNode = node.GetFirstChildElement("FileFilters").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_FileFilters.emplace_back(KLabeledValue(entryNode.GetValue(), V(entryNode.GetAttribute("Label"))));
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
KSaveManager* KSaveManagerConfig::GetManager() const
{
	return m_Manager;
}

KSMSaveFile* KSaveManagerConfig::QuerySaveFile(const wxString& fullPath) const
{
	if (GetSaveFileFormat() == "BethesdaMorrowind")
	{
		return new KSMSaveFileBethesdaMorrowind(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaOblivion")
	{
		return new KSMSaveFileBethesdaOblivion(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaSkyrim")
	{
		return new KSMSaveFileBethesdaSkyrim(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaSkyrimSE")
	{
		return new KSMSaveFileBethesdaSkyrimSE(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaFallout3")
	{
		return new KSMSaveFileBethesdaFallout3(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaFalloutNV")
	{
		return new KSMSaveFileBethesdaFalloutNV(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaFallout4")
	{
		return new KSMSaveFileBethesdaFallout4(fullPath);
	}
	if (GetSaveFileFormat() == "Sacred2")
	{
		//return new KSMSaveFileSacred2(fullPath);
	}
	return NULL;
}
