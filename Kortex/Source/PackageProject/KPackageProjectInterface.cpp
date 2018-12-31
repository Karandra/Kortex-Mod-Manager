#include "stdafx.h"
#include "KPackageProjectInterface.h"
#include "KPackageProject.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

KPPIImageEntry::KPPIImageEntry(const wxString& path, const wxString& description, bool isVisible)
	:m_Path(path), m_Description(description), m_IsVisiable(isVisible)
{
}
KPPIImageEntry::~KPPIImageEntry()
{
}

//////////////////////////////////////////////////////////////////////////
KPackageProjectInterface::KPackageProjectInterface(KPackageProject& project)
	:KPackageProjectPart(project)
{
}
KPackageProjectInterface::~KPackageProjectInterface()
{
}

const KPPIImageEntry* KPackageProjectInterface::FindEntryWithValue(const wxString& path) const
{
	auto it = std::find_if(m_Images.begin(), m_Images.end(), [path](const KPPIImageEntry& v)
	{
		return v.GetPath().IsSameAs(path, false);
	});
	return it != m_Images.end() ? &(*it) : nullptr;
}
KPPIImageEntry* KPackageProjectInterface::FindEntryWithValue(const wxString& path)
{
	auto it = std::find_if(m_Images.begin(), m_Images.end(), [path](const KPPIImageEntry& v)
	{
		return v.GetPath().IsSameAs(path, false);
	});
	return it != m_Images.end() ? &(*it) : nullptr;
}
