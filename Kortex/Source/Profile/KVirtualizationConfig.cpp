#include "stdafx.h"
#include "ConfigManager/KConfigManager.h"
#include "KVirtualizationConfig.h"
#include "KProfile.h"
#include "KApp.h"

//////////////////////////////////////////////////////////////////////////
KVirtualizationEntry::KVirtualizationEntry(KxXMLNode& node)
{
	m_Source = V(node.GetFirstChildElement("Source").GetValue());
	m_Target = V(node.GetFirstChildElement("Target").GetValue());
}

//////////////////////////////////////////////////////////////////////////
KxSingletonPtr_Define(KVirtualizationConfig);

KVirtualizationConfig::KVirtualizationConfig(KProfile& profile, KxXMLNode& node)
{
	auto ReadMirroredLocation = [](KVirtualizationEntry::Vector& array, const KxXMLNode& node, const wxString& name)
	{
		for (KxXMLNode entryNode = node.GetFirstChildElement(name).GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			if (!array.emplace_back(KVirtualizationEntry(entryNode)).IsOK())
			{
				array.pop_back();
			}
		}
	};
	auto ReadMandatoryLocation = [](KxStringVector& array, const KxXMLNode& node, const wxString& name)
	{
		for (KxXMLNode entryNode = node.GetFirstChildElement(name).GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			if (array.emplace_back(V(entryNode.GetValue())).IsEmpty())
			{
				array.pop_back();
			}
		}
	};

	ReadMirroredLocation(m_MirroredLocations, node, "MirroredLocations");
	ReadMandatoryLocation(m_MandatoryVirtualFolders, node, "MandatoryVirtualFolders");
}

size_t KVirtualizationConfig::GetEntriesCount(KPVEntryType index) const
{
	switch (index)
	{
		case KPVE_MIRRORED:
		{
			return m_MirroredLocations.size();
		}
	};
	return 0;
}
const KVirtualizationEntry* KVirtualizationConfig::GetEntryAt(KPVEntryType index, size_t i) const
{
	const KVirtualizationEntry::Vector* pArray = NULL;
	switch (index)
	{
		case KPVE_MIRRORED:
		{
			pArray = &m_MirroredLocations;
			break;
		}
	};

	if (pArray && i < pArray->size())
	{
		return &pArray->at(i);
	}
	return NULL;
}
