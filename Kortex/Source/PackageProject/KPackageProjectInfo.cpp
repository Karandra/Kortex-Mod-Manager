#include "stdafx.h"
#include "KPackageProjectInfo.h"
#include "KPackageProject.h"
#include "KApp.h"
#include "KAux.h"

KPackageProjectInfo::KPackageProjectInfo(KPackageProject& project)
	:KPackageProjectPart(project)
{
}
KPackageProjectInfo::~KPackageProjectInfo()
{
}

int64_t KPackageProjectInfo::GetWebSiteModID(KNetworkProviderID index) const
{
	return KModEntry::GetWebSiteModID(m_FixedWebSites, index);
}
bool KPackageProjectInfo::HasWebSite(KNetworkProviderID index) const
{
	return KModEntry::HasWebSite(m_FixedWebSites, index);
}
KLabeledValue KPackageProjectInfo::GetWebSite(KNetworkProviderID index) const
{
	return KModEntry::GetWebSite(m_FixedWebSites, index, GetProject().GetSignature());
}
void KPackageProjectInfo::SetWebSite(KNetworkProviderID index, int64_t modID)
{
	KModEntry::SetWebSite(m_FixedWebSites, index, modID);
}

void KPackageProjectInfo::ToggleTag(const wxString& value, bool set)
{
	KModEntry::ToggleTag(m_Tags, value, set);
}
