#include "stdafx.h"
#include "KPackageProject.h"
#include "PackageManager/KPackageManager.h"
#include "ModManager/KModManager.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"
#include "KAux.h"

void KPackageProject::CheckCondition(bool& currentStatus, int& cookie, bool isThis, KPPOperator operatorType)
{
	// If cookie is zero, then this is the first item
	if (cookie == 0)
	{
		currentStatus = isThis;
		cookie = 1;
	}
	else
	{
		if (operatorType == KPP_OPERATOR_OR)
		{
			currentStatus = isThis || currentStatus;
		}
		else
		{
			currentStatus = isThis && currentStatus;
		}
	}
}
void KPackageProject::CheckCondition(bool& currentStatus, int& cookie, const KPPRRequirementEntry& entry)
{
	CheckCondition(currentStatus, cookie, entry.GetOverallStatus(), entry.GetOperator());
}
void KPackageProject::CheckCondition(bool& currentStatus, int& cookie, bool isThis, const KPPCFlagEntry& entry)
{
	CheckCondition(currentStatus, cookie, isThis, entry.GetOperator());
}

KPackageProject::KPackageProject()
	:m_Config(*this),
	m_Info(*this),
	m_FileData(*this),
	m_Interface(*this),
	m_Requirements(*this),
	m_Components(*this),

	m_FormatVersion(KPackageManager::GetInstance()->GetVersion()),
	m_TargetProfileID(KApp::Get().GetCurrentGameID())
{
}
KPackageProject::~KPackageProject()
{
}

wxString KPackageProject::GetSignature() const
{
	return KModEntry::GetSignatureFromID(m_ModID);
}
wxString KPackageProject::ComputeModID() const
{
	// ID -> Name -> translated name -> project file name
	const wxString& id = GetModID();
	if (!id.IsEmpty())
	{
		return id;
	}
	else
	{
		const wxString& name = GetInfo().GetName();
		if (!name.IsEmpty())
		{
			return name;
		}
		else
		{
			const wxString& translatedName = GetInfo().GetTranslatedName();
			if (!translatedName.IsEmpty())
			{
				return translatedName;
			}
			else
			{
				wxString fileName = GetConfig().GetInstallPackageFile().AfterLast('\\').BeforeFirst('.');
				if (!fileName.IsEmpty())
				{
					return fileName;
				}
			}
		}
	}
	return wxEmptyString;
}
wxString KPackageProject::ComputeModName() const
{
	// Name -> translated name -> ID -> project file name
	const wxString& name = GetInfo().GetName();
	if (!name.IsEmpty())
	{
		return name;
	}
	else
	{
		const wxString& translatedName = GetInfo().GetTranslatedName();
		if (!translatedName.IsEmpty())
		{
			return translatedName;
		}
		else
		{
			const wxString& id = GetModID();
			if (!id.IsEmpty())
			{
				return id;
			}
			else
			{
				wxString fileName = GetConfig().GetInstallPackageFile().AfterLast('\\').BeforeFirst('.');
				if (!fileName.IsEmpty())
				{
					return fileName;
				}
			}
		}
	}
	return wxEmptyString;
}
wxString KPackageProject::GetLocation(KModManagerLocation index) const
{
	return KModManager::GetLocation(index, GetSignature());
}
