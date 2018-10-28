#include "stdafx.h"
#include "ConfigManager/KConfigManager.h"
#include "KVirtualizationConfig.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"

//////////////////////////////////////////////////////////////////////////
KVirtualizationMirroredEntry::KVirtualizationMirroredEntry(const KxXMLNode& parentNode)
{
	for (KxXMLNode node = parentNode.GetFirstChildElement("Sources").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		m_Sources.push_back(node.GetValue());
	}
	m_Target = parentNode.GetFirstChildElement("Target").GetValue();
}

KxStringVector KVirtualizationMirroredEntry::GetSources() const
{
	KxStringVector sources = m_Sources;
	std::transform(sources.begin(), sources.end(), sources.begin(), [](const wxString& s)
	{
		return KVarExp(s);
	});
	return sources;
}

wxString KVirtualizationMirroredEntry::GetSource() const
{
	return !m_Sources.empty() ? KVarExp(m_Sources.front()) : wxNullString;
}
wxString KVirtualizationMirroredEntry::GetTarget() const
{
	return KVarExp(m_Target);
}

//////////////////////////////////////////////////////////////////////////
KVirtualizationMandatoryEntry::KVirtualizationMandatoryEntry(const KxXMLNode& parentNode)
{
	// Folder path will not be expanded here
	m_Source = parentNode.GetValue();
	m_Name = KVarExp(parentNode.GetAttribute("Name"));
}

wxString KVirtualizationMandatoryEntry::GetSource() const
{
	return KVarExp(m_Source);
}
wxString KVirtualizationMandatoryEntry::GetName() const
{
	return KVarExp(m_Name);
}

//////////////////////////////////////////////////////////////////////////
KVirtualizationConfig::KVirtualizationConfig(KGameInstance& profile, const KxXMLNode& node)
{
	auto ReadEntries = [](auto& array, const KxXMLNode& rootNode, const wxString& name)
	{
		for (KxXMLNode node = rootNode.GetFirstChildElement(name).GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			if (!array.emplace_back(node).IsOK())
			{
				array.pop_back();
			}
		}
	};

	ReadEntries(m_MirroredLocations, node, "MirroredLocations");
	ReadEntries(m_MandatoryVirtualFolders, node, "MandatoryLocations");
}
