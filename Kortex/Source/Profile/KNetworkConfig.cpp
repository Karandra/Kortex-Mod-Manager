#include "stdafx.h"
#include "KNetworkConfig.h"
#include "Network/KNetwork.h"
#include "KApp.h"

KxSingletonPtr_Define(KNetworkConfig);

KNetworkConfig::KNetworkConfig(KProfile& profile, KxXMLNode& node)
{
	m_Network = new KNetwork();

	m_NexusID = V(node.GetFirstChildElement("NexusID").GetValue());
}
KNetworkConfig::~KNetworkConfig()
{
	delete m_Network;
}
