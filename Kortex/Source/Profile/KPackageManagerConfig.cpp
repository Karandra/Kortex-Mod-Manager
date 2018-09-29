#include "stdafx.h"
#include "KPackageManagerConfig.h"
#include "KProfile.h"
#include "PackageManager/KPackageManager.h"
#include "KApp.h"

KPackageManagerConfig::KPackageManagerConfig(KProfile& profile, KxXMLNode& node)
{
	m_Manager = new KPackageManager(node);
}
KPackageManagerConfig::~KPackageManagerConfig()
{
	delete m_Manager;
}
