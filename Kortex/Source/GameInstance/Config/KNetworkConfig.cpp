#include "stdafx.h"
#include "KNetworkConfig.h"
#include "Network/KNetwork.h"
#include "KApp.h"

KNetworkConfig::KNetworkConfig(KGameInstance& profile, const KxXMLNode& node)
{
	m_NexusID = KVarExp(node.GetFirstChildElement("NexusID").GetValue());
	m_SteamID = node.GetFirstChildElement("NexusID").GetValueInt(m_SteamID);
}
KNetworkConfig::~KNetworkConfig()
{
}
