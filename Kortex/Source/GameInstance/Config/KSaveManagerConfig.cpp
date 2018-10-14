#include "stdafx.h"
#include "KSaveManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "SaveManager/KSaveManager.h"
#include "SaveManager/KSaveFile.h"
#include "SaveManager/KSaveFileBethesdaMorrowind.h"
#include "SaveManager/KSaveFileBethesdaOblivion.h"
#include "SaveManager/KSaveFileBethesdaSkyrim.h"
#include "SaveManager/KSaveFileBethesdaSkyrimSE.h"
#include "SaveManager/KSaveFileBethesdaFallout3.h"
#include "SaveManager/KSaveFileBethesdaFalloutNV.h"
#include "SaveManager/KSaveFileBethesdaFallout4.h"
#include "KApp.h"
#include "KAux.h"

KSaveManagerConfig::KSaveManagerConfig(KGameInstance& profile, const KxXMLNode& node)
	:m_SaveFileFormat(node.GetAttribute("SaveFileFormat"))
{
	m_Location = node.GetFirstChildElement("Location").GetValue();

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

KSaveFile* KSaveManagerConfig::QuerySaveFile(const wxString& fullPath) const
{
	if (GetSaveFileFormat() == "BethesdaMorrowind")
	{
		return new KSaveFileBethesdaMorrowind(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaOblivion")
	{
		return new KSaveFileBethesdaOblivion(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaSkyrim")
	{
		return new KSaveFileBethesdaSkyrim(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaSkyrimSE")
	{
		return new KSaveFileBethesdaSkyrimSE(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaFallout3")
	{
		return new KSaveFileBethesdaFallout3(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaFalloutNV")
	{
		return new KSaveFileBethesdaFalloutNV(fullPath);
	}
	if (GetSaveFileFormat() == "BethesdaFallout4")
	{
		return new KSaveFileBethesdaFallout4(fullPath);
	}
	if (GetSaveFileFormat() == "Sacred2")
	{
		//return new KSaveFileSacred2(fullPath);
	}
	return NULL;
}

wxString KSaveManagerConfig::GetSaveFileFormat() const
{
	return m_SaveFileFormat;
}
wxString KSaveManagerConfig::GetLocation() const
{
	return V(m_Location);
}
