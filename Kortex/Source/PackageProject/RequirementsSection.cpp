#include "stdafx.h"
#include "RequirementsSection.h"
#include "ModPackageProject.h"
#include "ModPackages/IPackageManager.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

namespace
{
	namespace ObjFuncConst
	{
		constexpr const auto NONE_STRING = wxS("");
		constexpr const auto MOD_ACTIVE_STRING = wxS("ModActive");
		constexpr const auto MOD_INACTIVE_STRING = wxS("ModInactive");
		constexpr const auto PLUGIN_ACTIVE_STRING = wxS("PluginActive");
		constexpr const auto PLUGIN_INACTIVE_STRING = wxS("PluginInactive");
		constexpr const auto FILE_EXIST_STRING = wxS("FileExist");
		constexpr const auto FILE_NOT_EXIST_STRING = wxS("FileNotExist");
	}
	namespace TypeConst
	{
		constexpr const auto USER_STRING = wxS("User");
		constexpr const auto SYSTEM_STRING = wxS("System");
		constexpr const auto AUTO_STRING = wxS("Auto");
	}
}

namespace Kortex::PackageProject
{
	RequirementItem::RequirementItem(ReqType typeDescriptor)
		:m_ObjectFunction(RequirementsSection::ms_DefaultObjectFunction),
		m_RequiredVersionFunction(RequirementsSection::ms_DefaultVersionOperator),
		m_TypeDescriptor(typeDescriptor)
	{
	}
	RequirementItem::~RequirementItem()
	{
	}
	
	const KxVersion& RequirementItem::GetCurrentVersion() const
	{
		if (!m_CurrentVersionChecked)
		{
			m_CurrentVersion = Kortex::IPackageManager::GetInstance()->GetRequirementVersion(this);
			m_CurrentVersionChecked = true;
		}
		return m_CurrentVersion;
	}
	void RequirementItem::ResetCurrentVersion()
	{
		m_CurrentVersion = KxNullVersion;
		m_CurrentVersionChecked = false;
	}
	bool RequirementItem::CheckVersion() const
	{
		return RequirementsSection::CompareVersions(GetRVFunction(), GetCurrentVersion(), GetRequiredVersion());
	}
	
	ReqState RequirementItem::GetObjectFunctionResult() const
	{
		if (!m_ObjectFunctionResultChecked)
		{
			m_ObjectFunctionResult = Kortex::IPackageManager::GetInstance()->CheckRequirementState(this);
			m_ObjectFunctionResultChecked = true;
		}
		return m_ObjectFunctionResult;
	}
	void RequirementItem::ResetObjectFunctionResult()
	{
		m_ObjectFunctionResult = ReqState::Unknown;
		m_ObjectFunctionResultChecked = false;
	}
	
	bool RequirementItem::IsStd() const
	{
		return Kortex::IPackageManager::GetInstance()->FindStdReqirement(GetID()) != nullptr;
	}
	bool RequirementItem::IsSystem() const
	{
		switch (m_TypeDescriptor)
		{
			case ReqType::System:
			{
				return true;
			}
			case ReqType::User:
			{
				return false;
			}
			case ReqType::Auto:
			{
				return IsStd();
			}
		};
		return false;
	}
	bool RequirementItem::IsUserEditable() const
	{
		return !IsSystem();
	}
	
	void RequirementItem::TrySetTypeDescriptor(ReqType type)
	{
		switch (type)
		{
			case ReqType::User:
			case ReqType::Auto:
			{
				m_TypeDescriptor = type;
				break;
			}
			case ReqType::System:
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
	bool RequirementItem::ConformToTypeDescriptor()
	{
		if (m_TypeDescriptor == ReqType::System || m_TypeDescriptor == ReqType::Auto)
		{
			const RequirementItem* stdEntry = Kortex::IPackageManager::GetInstance()->FindStdReqirement(GetID());
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
	
	bool RequirementItem::CalcOverallStatus()
	{
		if (!m_OverallStatusCalculated)
		{
			m_OverallStatus = CheckVersion() && GetObjectFunctionResult() == ReqState::True;
			m_OverallStatusCalculated = true;
		}
		return m_OverallStatus;
	}
}

namespace Kortex::PackageProject
{
	wxString RequirementGroup::GetFlagNamePrefix()
	{
		return "REQFLAG_";
	}
	wxString RequirementGroup::GetFlagName(const wxString& id)
	{
		return GetFlagNamePrefix() + id;
	}
	
	RequirementGroup::RequirementGroup()
		:m_Operator(RequirementsSection::ms_DefaultGroupOperator)
	{
	}
	RequirementGroup::~RequirementGroup()
	{
	}
	
	RequirementItem* RequirementGroup::FindEntry(const wxString& id) const
	{
		auto it = std::find_if(m_Entries.cbegin(), m_Entries.cend(), [id](const RequirementItem::Vector::value_type& entry)
		{
			return entry->GetID() == id;
		});
	
		if (it != m_Entries.cend())
		{
			return it->get();
		}
		return nullptr;
	}
	
	bool RequirementGroup::CalcGroupStatus()
	{
		if (!m_GroupStatusCalculated)
		{
			ConditionChecker checker;
			for (auto& entry: m_Entries)
			{
				checker(entry->CalcOverallStatus(), m_Operator);
			}
	
			m_GroupStatus = checker.GetResult();
			m_GroupStatusCalculated = true;
		}
		return m_GroupStatus;
	}
}

namespace Kortex::PackageProject
{
	ObjectFunction RequirementsSection::StringToObjectFunction(const wxString& name)
	{
		if (name == ObjFuncConst::NONE_STRING)
		{
			return ObjectFunction::None;
		}
		if (name == ObjFuncConst::MOD_ACTIVE_STRING)
		{
			return ObjectFunction::ModActive;
		}
		if (name == ObjFuncConst::MOD_INACTIVE_STRING)
		{
			return ObjectFunction::ModInactive;
		}
		if (name == ObjFuncConst::PLUGIN_ACTIVE_STRING)
		{
			return ObjectFunction::PluginActive;
		}
		if (name == ObjFuncConst::PLUGIN_INACTIVE_STRING)
		{
			return ObjectFunction::PluginInactive;
		}
		if (name == ObjFuncConst::FILE_EXIST_STRING)
		{
			return ObjectFunction::FileExist;
		}
		if (name == ObjFuncConst::FILE_NOT_EXIST_STRING)
		{
			return ObjectFunction::FileNotExist;
		}
		return ms_DefaultObjectFunction;
	}
	wxString RequirementsSection::ObjectFunctionToString(ObjectFunction state)
	{
		switch (state)
		{
			case ObjectFunction::None:
			{
				return ObjFuncConst::NONE_STRING;
			}
			case ObjectFunction::ModActive:
			{
				return ObjFuncConst::MOD_ACTIVE_STRING;
			}
			case ObjectFunction::ModInactive:
			{
				return ObjFuncConst::MOD_INACTIVE_STRING;
			}
			case ObjectFunction::PluginActive:
			{
				return ObjFuncConst::PLUGIN_ACTIVE_STRING;
			}
			case ObjectFunction::PluginInactive:
			{
				return ObjFuncConst::PLUGIN_INACTIVE_STRING;
			}
			case ObjectFunction::FileExist:
			{
				return ObjFuncConst::FILE_EXIST_STRING;
			}
			case ObjectFunction::FileNotExist:
			{
				return ObjFuncConst::FILE_NOT_EXIST_STRING;
			}
		};
		return wxEmptyString;
	}
	
	ReqType RequirementsSection::StringToTypeDescriptor(const wxString& name)
	{
		if (name == TypeConst::USER_STRING)
		{
			return ReqType::User;
		}
		if (name == TypeConst::SYSTEM_STRING)
		{
			return ReqType::System;
		}
		if (name == TypeConst::AUTO_STRING)
		{
			return ReqType::Auto;
		}
		return ms_DefaultTypeDescriptor;
	}
	wxString RequirementsSection::TypeDescriptorToString(ReqType type)
	{
		switch (type)
		{
			case ReqType::User:
			{
				return TypeConst::USER_STRING;
			}
			case ReqType::System:
			{
				return TypeConst::SYSTEM_STRING;
			}
			case ReqType::Auto:
			{
				return TypeConst::AUTO_STRING;
			}
		};
		return wxEmptyString;
	}
	
	bool RequirementsSection::CompareVersions(Operator operatorType, const KxVersion& current, const KxVersion& required)
	{
		// Always return true when requested to compare against invalid (or unspecified) version.
		// Or operator is unspecified, which should mean that caller is not interested in this check.
		if (operatorType == Operator::None || !required.IsOK())
		{
			return true;
		}
	
		switch (operatorType)
		{
			case Operator::Equal:
			{
				return current == required;
			}
			case Operator::NotEqual:
			{
				return current != required;
			}
			case Operator::GreaterThan:
			{
				return current > required;
			}
			case Operator::GreaterThanOrEqual:
			{
				return current >= required;
			}
			case Operator::LessThan:
			{
				return current < required;
			}
			case Operator::LessThanOrEqual:
			{
				return current <= required;
			}
		};
	
		// Any other unsupported operator, return false.
		return false;
	}
	
	RequirementsSection::RequirementsSection(ModPackageProject& project)
		:ProjectSection(project)
	{
	}
	RequirementsSection::~RequirementsSection()
	{
	}
	
	RequirementGroup* RequirementsSection::FindGroupWithID(const wxString& id) const
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
	
	bool RequirementsSection::IsDefaultGroupContains(const wxString& groupID) const
	{
		auto it = std::find_if(m_DefaultGroup.begin(), m_DefaultGroup.end(), [&groupID](const wxString& id)
		{
			return id == groupID;
		});
		return it != m_DefaultGroup.end();
	}
	KxStringVector RequirementsSection::GetFlagNames() const
	{
		KxStringVector flagNames;
		for (const auto& group: m_Groups)
		{
			flagNames.push_back(group->GetFlagName());
		}
		return flagNames;
	}
	bool RequirementsSection::CalcOverallStatus(const KxStringVector& groups) const
	{
		ConditionChecker checker;
		for (const wxString& id: groups)
		{
			RequirementGroup* group = FindGroupWithID(id);
			if (group)
			{
				checker(group->CalcGroupStatus(), Operator::And);
			}
		}
		return checker.GetResult();
	}
}
