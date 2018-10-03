#include "stdafx.h"
#include "KPackageProjectRequirements.h"
#include "KPackageProject.h"
#include "PackageManager/KPackageManager.h"
#include "KApp.h"
#include "KAux.h"

KPPRRequirementEntry::KPPRRequirementEntry(KPPRTypeDescriptor typeDescriptor)
	:m_Operator(KPackageProjectRequirements::ms_DefaultEntryOperator),
	m_ObjectFunction(KPackageProjectRequirements::ms_DefaultObjectFunction),
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
		m_CurrentVersion = KPackageManager::GetInstance()->GetRequirementVersion(this);
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
		m_ObjectFunctionResult = KPackageManager::GetInstance()->CheckRequirementState(this);
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
	return KPackageManager::GetInstance()->IsStdReqirement(GetID());
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
		const KPPRRequirementEntry* stdEntry = KPackageManager::GetInstance()->FindStdReqirement(GetID());
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
{
}
KPPRRequirementsGroup::~KPPRRequirementsGroup()
{
}

KPPRRequirementEntry* KPPRRequirementsGroup::FindEntry(const wxString& id) const
{
	auto it = std::find_if(m_Entries.cbegin(), m_Entries.cend(), [id](const KPPRRequirementEntryArray::value_type& entry)
	{
		return entry->GetID() == id;
	});

	if (it != m_Entries.cend())
	{
		return it->get();
	}
	return NULL;
}

bool KPPRRequirementsGroup::CalcGroupStatus()
{
	// Sequential variant
	if (!m_GroupStatusCalculated)
	{
		bool overallValue = true;
		int cookie = 0;

		for (auto& entry: m_Entries)
		{
			entry->CalcOverallStatus();
			KPackageProject::CheckCondition(overallValue, cookie, *entry);
		}

		m_GroupStatus = overallValue;
		m_GroupStatusCalculated = true;
	}
	return m_GroupStatus;
}

//////////////////////////////////////////////////////////////////////////
#define KPPR_OPERATOR_NONE_STRING			""
#define KPPR_OPERATOR_EQ_STRING				"EQ"
#define KPPR_OPERATOR_NOT_EQ_STRING			"NOTEQ"
#define KPPR_OPERATOR_LT_STRING				"LT"
#define KPPR_OPERATOR_LTEQ_STRING			"LTEQ"
#define KPPR_OPERATOR_GT_STRING				"GT"
#define KPPR_OPERATOR_GTEQ_STRING			"GTEQ"
#define KPPR_OPERATOR_AND_STRING			"AND"
#define KPPR_OPERATOR_OR_STRING				"OR"

#define KPPR_OBJFUNC_NONE_STRING			""
#define KPPR_OBJFUNC_MOD_ACTIVE_STRING		"ModActive"
#define KPPR_OBJFUNC_MOD_INACTIVE_STRING	"ModIncative"
#define KPPR_OBJFUNC_PLUGIN_ACTIVE_STRING	"PluginActive"
#define KPPR_OBJFUNC_PLUGIN_INACTIVE_STRING	"PluginIncative"
#define KPPR_OBJFUNC_FILE_EXIST_STRING		"FileExist"
#define KPPR_OBJFUNC_FILE_NOT_EXIST_STRING	"FileNotExist"

#define KPPR_TYPE_USER_STRING				"User"
#define KPPR_TYPE_SYSTEM_STRING				"System"
#define KPPR_TYPE_AUTO_STRING				"Auto"

wxString KPackageProjectRequirements::OperatorToSymbolicName(KPPOperator operatorType)
{
	switch (operatorType)
	{
		case KPP_OPERATOR_EQ:
		{
			return "==";
		}
		case KPP_OPERATOR_NOT_EQ:
		{
			return "!=";
		}
		case KPP_OPERATOR_LT:
		{
			return "<";
		}
		case KPP_OPERATOR_LTEQ:
		{
			return "<=";
		}
		case KPP_OPERATOR_GT:
		{
			return ">";
		}
		case KPP_OPERATOR_GTEQ:
		{
			return ">=";
		}
		case KPP_OPERATOR_AND:
		{
			return "&&";
		}
		case KPP_OPERATOR_OR:
		{
			return "||";
		}
	};
	return wxEmptyString;
}
wxString KPackageProjectRequirements::OperatorToString(KPPOperator operatorType)
{
	switch (operatorType)
	{
		case KPP_OPERATOR_EQ:
		{
			return KPPR_OPERATOR_EQ_STRING;
		}
		case KPP_OPERATOR_NOT_EQ:
		{
			return KPPR_OPERATOR_NOT_EQ_STRING;
		}
		case KPP_OPERATOR_LT:
		{
			return KPPR_OPERATOR_LT_STRING;
		}
		case KPP_OPERATOR_LTEQ:
		{
			return KPPR_OPERATOR_LTEQ_STRING;
		}
		case KPP_OPERATOR_GT:
		{
			return KPPR_OPERATOR_GT_STRING;
		}
		case KPP_OPERATOR_GTEQ:
		{
			return KPPR_OPERATOR_GTEQ_STRING;
		}
		case KPP_OPERATOR_AND:
		{
			return KPPR_OPERATOR_AND_STRING;
		}
		case KPP_OPERATOR_OR:
		{
			return KPPR_OPERATOR_OR_STRING;
		}
		case KPP_OPERATOR_NONE:
		{
			return KPPR_OPERATOR_NONE_STRING;
		}
	};
	return wxEmptyString;
}
KPPOperator KPackageProjectRequirements::StringToOperator(const wxString& name, bool allowNone, KPPOperator default)
{
	if (name == KPPR_OPERATOR_EQ_STRING)
	{
		return KPP_OPERATOR_EQ;
	}
	if (name == KPPR_OPERATOR_NOT_EQ_STRING)
	{
		return KPP_OPERATOR_NOT_EQ;
	}
	if (name == KPPR_OPERATOR_LT_STRING)
	{
		return KPP_OPERATOR_LT;
	}
	if (name == KPPR_OPERATOR_LTEQ_STRING)
	{
		return KPP_OPERATOR_LTEQ;
	}
	if (name == KPPR_OPERATOR_GT_STRING)
	{
		return KPP_OPERATOR_GT;
	}
	if (name == KPPR_OPERATOR_GTEQ_STRING)
	{
		return KPP_OPERATOR_GTEQ;
	}
	if (name == KPPR_OPERATOR_AND_STRING)
	{
		return KPP_OPERATOR_AND;
	}
	if (name == KPPR_OPERATOR_OR_STRING)
	{
		return KPP_OPERATOR_OR;
	}
	if (allowNone && name == KPPR_OPERATOR_NONE_STRING)
	{
		return KPP_OPERATOR_NONE;
	}
	return default;
}

KPPRObjectFunction KPackageProjectRequirements::StringToObjectFunction(const wxString& name)
{
	if (name == KPPR_OBJFUNC_NONE_STRING)
	{
		return KPPR_OBJFUNC_NONE;
	}
	if (name == KPPR_OBJFUNC_MOD_ACTIVE_STRING)
	{
		return KPPR_OBJFUNC_MOD_ACTIVE;
	}
	if (name == KPPR_OBJFUNC_MOD_INACTIVE_STRING)
	{
		return KPPR_OBJFUNC_MOD_INACTIVE;
	}
	if (name == KPPR_OBJFUNC_PLUGIN_ACTIVE_STRING)
	{
		return KPPR_OBJFUNC_PLUGIN_ACTIVE;
	}
	if (name == KPPR_OBJFUNC_PLUGIN_INACTIVE_STRING)
	{
		return KPPR_OBJFUNC_PLUGIN_INACTIVE;
	}
	if (name == KPPR_OBJFUNC_FILE_EXIST_STRING)
	{
		return KPPR_OBJFUNC_FILE_EXIST;
	}
	if (name == KPPR_OBJFUNC_FILE_NOT_EXIST_STRING)
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
			return KPPR_OBJFUNC_NONE_STRING;
		}
		case KPPR_OBJFUNC_MOD_ACTIVE:
		{
			return KPPR_OBJFUNC_MOD_ACTIVE_STRING;
		}
		case KPPR_OBJFUNC_MOD_INACTIVE:
		{
			return KPPR_OBJFUNC_MOD_INACTIVE_STRING;
		}
		case KPPR_OBJFUNC_PLUGIN_ACTIVE:
		{
			return KPPR_OBJFUNC_PLUGIN_ACTIVE_STRING;
		}
		case KPPR_OBJFUNC_PLUGIN_INACTIVE:
		{
			return KPPR_OBJFUNC_PLUGIN_INACTIVE_STRING;
		}
		case KPPR_OBJFUNC_FILE_EXIST:
		{
			return KPPR_OBJFUNC_FILE_EXIST_STRING;
		}
		case KPPR_OBJFUNC_FILE_NOT_EXIST:
		{
			return KPPR_OBJFUNC_FILE_NOT_EXIST_STRING;
		}
	};
	return wxEmptyString;
}

KPPRTypeDescriptor KPackageProjectRequirements::StringToTypeDescriptor(const wxString& name)
{
	if (name == KPPR_TYPE_USER_STRING)
	{
		return KPPR_TYPE_USER;
	}
	if (name == KPPR_TYPE_SYSTEM_STRING)
	{
		return KPPR_TYPE_SYSTEM;
	}
	if (name == KPPR_TYPE_AUTO_STRING)
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
			return KPPR_TYPE_USER_STRING;
		}
		case KPPR_TYPE_SYSTEM:
		{
			return KPPR_TYPE_SYSTEM_STRING;
		}
		case KPPR_TYPE_AUTO:
		{
			return KPPR_TYPE_AUTO_STRING;
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
	return NULL;
}

bool KPackageProjectRequirements::IsDefaultGroupContains(const wxString& groupID) const
{
	auto it = std::find_if(m_DefaultGroup.cbegin(), m_DefaultGroup.cend(), [&groupID](const wxString& id)
	{
		return id == groupID;
	});
	return it != m_DefaultGroup.cend();
}
KxStringVector KPackageProjectRequirements::GetFlagNames() const
{
	KxStringVector tFlagNames;
	for (const auto& pSet: m_Groups)
	{
		tFlagNames.push_back(pSet->GetFlagName());
	}
	return tFlagNames;
}
bool KPackageProjectRequirements::CalcOverallStatus(const KxStringVector& groups) const
{
	int value = -1;
	for (const wxString& id: groups)
	{
		KPPRRequirementsGroup* group = FindGroupWithID(id);
		if (group)
		{
			bool bResult = group->CalcGroupStatus();
			if (value == -1)
			{
				value = bResult;
			}
			else
			{
				value = bResult ? (bool)value && bResult : 0;
			}
		}
	}
	return value == 1;
}