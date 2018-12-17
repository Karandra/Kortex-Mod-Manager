#include "stdafx.h"
#include "KPackageProjectRequirements.h"
#include "KPackageProject.h"
#include "PackageManager/KPackageManager.h"
#include <Kortex/Application.hpp>
#include "KAux.h"

KPPRRequirementEntry::KPPRRequirementEntry(KPPRTypeDescriptor typeDescriptor)
	:m_ObjectFunction(KPackageProjectRequirements::ms_DefaultObjectFunction),
	m_RequiredVersionFunction(KPackageProjectRequirements::ms_DefaultVersionOperator),
	m_TypeDescriptor(typeDescriptor)
{
}
KPPRRequirementEntry::~KPPRRequirementEntry()
{
}

const KxVersion& KPPRRequirementEntry::GetCurrentVersion() const
{
	if (!m_CurrentVersionChecked)
	{
		m_CurrentVersion = Kortex::KPackageManager::GetInstance()->GetRequirementVersion(this);
		m_CurrentVersionChecked = true;
	}
	return m_CurrentVersion;
}
void KPPRRequirementEntry::ResetCurrentVersion()
{
	m_CurrentVersion = KxNullVersion;
	m_CurrentVersionChecked = false;
}
bool KPPRRequirementEntry::CheckVersion() const
{
	return KPackageProjectRequirements::CompareVersions(GetRVFunction(), GetCurrentVersion(), GetRequiredVersion());
}

KPPReqState KPPRRequirementEntry::GetObjectFunctionResult() const
{
	if (!m_ObjectFunctionResultChecked)
	{
		m_ObjectFunctionResult = Kortex::KPackageManager::GetInstance()->CheckRequirementState(this);
		m_ObjectFunctionResultChecked = true;
	}
	return m_ObjectFunctionResult;
}
void KPPRRequirementEntry::ResetObjectFunctionResult()
{
	m_ObjectFunctionResult = KPPReqState::Unknown;
	m_ObjectFunctionResultChecked = false;
}

bool KPPRRequirementEntry::IsStd() const
{
	return Kortex::KPackageManager::GetInstance()->IsStdReqirement(GetID());
}
bool KPPRRequirementEntry::IsSystem() const
{
	switch (m_TypeDescriptor)
	{
		case KPPR_TYPE_SYSTEM:
		{
			return true;
		}
		case KPPR_TYPE_USER:
		{
			return false;
		}
		case KPPR_TYPE_AUTO:
		{
			return IsStd();
		}
	};
	return false;
}
bool KPPRRequirementEntry::IsUserEditable() const
{
	return !IsSystem();
}

void KPPRRequirementEntry::TrySetTypeDescriptor(KPPRTypeDescriptor type)
{
	switch (type)
	{
		case KPPR_TYPE_USER:
		case KPPR_TYPE_AUTO:
		{
			m_TypeDescriptor = type;
			break;
		}
		case KPPR_TYPE_SYSTEM:
		{
			if (IsStd())
			{
				m_TypeDescriptor = type;
			}
			break;
		}
	};
	ConformToTypeDescriptor();
}
bool KPPRRequirementEntry::ConformToTypeDescriptor()
{
	if (m_TypeDescriptor == KPPR_TYPE_SYSTEM || m_TypeDescriptor == KPPR_TYPE_AUTO)
	{
		const KPPRRequirementEntry* stdEntry = Kortex::KPackageManager::GetInstance()->FindStdReqirement(GetID());
		if (stdEntry)
		{
			SetName(stdEntry->GetName());
			SetObject(stdEntry->GetObject());
			SetObjectFunction(stdEntry->GetObjectFunction());

			ResetObjectFunctionResult();
			ResetCurrentVersion();
			return true;
		}
	}
	return false;
}

bool KPPRRequirementEntry::CalcOverallStatus()
{
	if (!m_OverallStatusCalculated)
	{
		m_OverallStatus = CheckVersion() && GetObjectFunctionResult() == KPPReqState::True;
		m_OverallStatusCalculated = true;
	}
	return m_OverallStatus;
}

//////////////////////////////////////////////////////////////////////////
wxString KPPRRequirementsGroup::GetFlagNamePrefix()
{
	return "REQFLAG_";
}
wxString KPPRRequirementsGroup::GetFlagName(const wxString& id)
{
	return GetFlagNamePrefix() + id;
}

KPPRRequirementsGroup::KPPRRequirementsGroup()
	:m_Operator(KPackageProjectRequirements::ms_DefaultGroupOperator)
{
}
KPPRRequirementsGroup::~KPPRRequirementsGroup()
{
}

KPPRRequirementEntry* KPPRRequirementsGroup::FindEntry(const wxString& id) const
{
	auto it = std::find_if(m_Entries.cbegin(), m_Entries.cend(), [id](const KPPRRequirementEntry::Vector::value_type& entry)
	{
		return entry->GetID() == id;
	});

	if (it != m_Entries.cend())
	{
		return it->get();
	}
	return nullptr;
}

bool KPPRRequirementsGroup::CalcGroupStatus()
{
	if (!m_GroupStatusCalculated)
	{
		KPackageProjectConditionChecker checker;
		for (auto& entry: m_Entries)
		{
			checker(entry->CalcOverallStatus(), m_Operator);
		}

		m_GroupStatus = checker.GetResult();
		m_GroupStatusCalculated = true;
	}
	return m_GroupStatus;
}

//////////////////////////////////////////////////////////////////////////
namespace ObjFuncConst
{
	constexpr const auto NONE_STRING = wxS("");
	constexpr const auto MOD_ACTIVE_STRING = wxS("ModActive");
	constexpr const auto MOD_INACTIVE_STRING = wxS("ModIncative");
	constexpr const auto PLUGIN_ACTIVE_STRING = wxS("PluginActive");
	constexpr const auto PLUGIN_INACTIVE_STRING = wxS("PluginIncative");
	constexpr const auto FILE_EXIST_STRING = wxS("FileExist");
	constexpr const auto FILE_NOT_EXIST_STRING = wxS("FileNotExist");
}
namespace TypeConst
{
	constexpr const auto USER_STRING = wxS("User");
	constexpr const auto SYSTEM_STRING = wxS("System");
	constexpr const auto AUTO_STRING = wxS("Auto");
}

//////////////////////////////////////////////////////////////////////////
KPPRObjectFunction KPackageProjectRequirements::StringToObjectFunction(const wxString& name)
{
	if (name == ObjFuncConst::NONE_STRING)
	{
		return KPPR_OBJFUNC_NONE;
	}
	if (name == ObjFuncConst::MOD_ACTIVE_STRING)
	{
		return KPPR_OBJFUNC_MOD_ACTIVE;
	}
	if (name == ObjFuncConst::MOD_INACTIVE_STRING)
	{
		return KPPR_OBJFUNC_MOD_INACTIVE;
	}
	if (name == ObjFuncConst::PLUGIN_ACTIVE_STRING)
	{
		return KPPR_OBJFUNC_PLUGIN_ACTIVE;
	}
	if (name == ObjFuncConst::PLUGIN_INACTIVE_STRING)
	{
		return KPPR_OBJFUNC_PLUGIN_INACTIVE;
	}
	if (name == ObjFuncConst::FILE_EXIST_STRING)
	{
		return KPPR_OBJFUNC_FILE_EXIST;
	}
	if (name == ObjFuncConst::FILE_NOT_EXIST_STRING)
	{
		return KPPR_OBJFUNC_FILE_NOT_EXIST;
	}
	return ms_DefaultObjectFunction;
}
wxString KPackageProjectRequirements::ObjectFunctionToString(KPPRObjectFunction state)
{
	switch (state)
	{
		case KPPR_OBJFUNC_NONE:
		{
			return ObjFuncConst::NONE_STRING;
		}
		case KPPR_OBJFUNC_MOD_ACTIVE:
		{
			return ObjFuncConst::MOD_ACTIVE_STRING;
		}
		case KPPR_OBJFUNC_MOD_INACTIVE:
		{
			return ObjFuncConst::MOD_INACTIVE_STRING;
		}
		case KPPR_OBJFUNC_PLUGIN_ACTIVE:
		{
			return ObjFuncConst::PLUGIN_ACTIVE_STRING;
		}
		case KPPR_OBJFUNC_PLUGIN_INACTIVE:
		{
			return ObjFuncConst::PLUGIN_INACTIVE_STRING;
		}
		case KPPR_OBJFUNC_FILE_EXIST:
		{
			return ObjFuncConst::FILE_EXIST_STRING;
		}
		case KPPR_OBJFUNC_FILE_NOT_EXIST:
		{
			return ObjFuncConst::FILE_NOT_EXIST_STRING;
		}
	};
	return wxEmptyString;
}

KPPRTypeDescriptor KPackageProjectRequirements::StringToTypeDescriptor(const wxString& name)
{
	if (name == TypeConst::USER_STRING)
	{
		return KPPR_TYPE_USER;
	}
	if (name == TypeConst::SYSTEM_STRING)
	{
		return KPPR_TYPE_SYSTEM;
	}
	if (name == TypeConst::AUTO_STRING)
	{
		return KPPR_TYPE_AUTO;
	}
	return ms_DefaultTypeDescriptor;
}
wxString KPackageProjectRequirements::TypeDescriptorToString(KPPRTypeDescriptor type)
{
	switch (type)
	{
		case KPPR_TYPE_USER:
		{
			return TypeConst::USER_STRING;
		}
		case KPPR_TYPE_SYSTEM:
		{
			return TypeConst::SYSTEM_STRING;
		}
		case KPPR_TYPE_AUTO:
		{
			return TypeConst::AUTO_STRING;
		}
	};
	return wxEmptyString;
}

bool KPackageProjectRequirements::CompareVersions(KPPOperator operatorType, const KxVersion& current, const KxVersion& required)
{
	// Always return true when requested to compare against invalid (or unspecified) version.
	// Or operator is unspecified, which should mean that caller is not interested in this check.
	if (operatorType == KPP_OPERATOR_NONE || !required.IsOK())
	{
		return true;
	}

	switch (operatorType)
	{
		case KPP_OPERATOR_EQ:
		{
			return current == required;
		}
		case KPP_OPERATOR_NOT_EQ:
		{
			return current != required;
		}
		case KPP_OPERATOR_GT:
		{
			return current > required;
		}
		case KPP_OPERATOR_GTEQ:
		{
			return current >= required;
		}
		case KPP_OPERATOR_LT:
		{
			return current < required;
		}
		case KPP_OPERATOR_LTEQ:
		{
			return current <= required;
		}
	};

	// Any other unsupported operator, return false.
	return false;
}

KPackageProjectRequirements::KPackageProjectRequirements(KPackageProject& project)
	:KPackageProjectPart(project)
{
}
KPackageProjectRequirements::~KPackageProjectRequirements()
{
}

KPPRRequirementsGroup* KPackageProjectRequirements::FindGroupWithID(const wxString& id) const
{
	auto it = std::find_if(m_Groups.cbegin(), m_Groups.cend(), [id](const auto& entry)
	{
		return entry->GetID() == id;
	});

	if (it != m_Groups.cend())
	{
		return it->get();
	}
	return nullptr;
}

bool KPackageProjectRequirements::IsDefaultGroupContains(const wxString& groupID) const
{
	auto it = std::find_if(m_DefaultGroup.begin(), m_DefaultGroup.end(), [&groupID](const wxString& id)
	{
		return id == groupID;
	});
	return it != m_DefaultGroup.end();
}
KxStringVector KPackageProjectRequirements::GetFlagNames() const
{
	KxStringVector flagNames;
	for (const auto& group: m_Groups)
	{
		flagNames.push_back(group->GetFlagName());
	}
	return flagNames;
}
bool KPackageProjectRequirements::CalcOverallStatus(const KxStringVector& groups) const
{
	KPackageProjectConditionChecker checker;
	for (const wxString& id: groups)
	{
		KPPRRequirementsGroup* group = FindGroupWithID(id);
		if (group)
		{
			checker(group->CalcGroupStatus(), KPP_OPERATOR_AND);
		}
	}
	return checker.GetResult();
}
