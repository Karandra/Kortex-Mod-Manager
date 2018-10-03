#include "stdafx.h"
#include "KNetworkConfig.h"
#include "Network/KNetwork.h"
#include "KApp.h"

KNetworkConfig::KNetworkConfig(KProfile& profile, const KxXMLNode& node)
{
	m_Network = new KNetwork();

	m_NexusID = V(node.GetFirstChildElement("NexusID").GetValue());
	m_SteamID = node.GetFirstChildElement("NexusID").GetValueInt(m_SteamID);
}
KNetworkConfig::~KNetworkConfig()
{
	delete m_Network;
}
