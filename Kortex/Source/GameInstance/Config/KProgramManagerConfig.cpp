#include "stdafx.h"
#include "KProgramManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"
#include "KAux.h"

KProgramManagerConfig::KProgramManagerConfig(KGameInstance& profile, const KxXMLNode& rootNode)
{
	for (KxXMLNode node = rootNode.GetFirstChildElement("Programs").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		if (!m_Entries.emplace_back(KProgramEntry(node)).IsOK())
		{
			m_Entries.pop_back();
		}
	}
}
