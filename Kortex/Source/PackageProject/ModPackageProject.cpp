#include "stdafx.h"
#include "ModPackageProject.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/PackageManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

namespace Kortex::PackageProject
{
	namespace OperatorConst
	{
		constexpr auto NONE = wxS("");
		constexpr auto EQ = wxS("==");
		constexpr auto NOT_EQ = wxS("!=");
		constexpr auto LT = wxS("<");
		constexpr auto LTEQ = wxS("<=");
		constexpr auto GT = wxS(">");
		constexpr auto GTEQ = wxS(">=");
		constexpr auto AND = wxS("&&");
		constexpr auto OR = wxS("||");

		constexpr auto NONE_STRING = wxS("");
		constexpr auto EQ_STRING = wxS("EQ");
		constexpr auto NOT_EQ_STRING = wxS("NOTEQ");
		constexpr auto LT_STRING = wxS("LT");
		constexpr auto LTEQ_STRING = wxS("LTEQ");
		constexpr auto GT_STRING = wxS("GT");
		constexpr auto GTEQ_STRING = wxS("GTEQ");
		constexpr auto AND_STRING = wxS("AND");
		constexpr auto OR_STRING = wxS("OR");
	}

	KxStringVector CreateOperatorList(const std::initializer_list<PackageProject::Operator>& operators, bool names)
	{
		KxStringVector list;
		list.reserve(operators.size());

		for (PackageProject::Operator value : operators)
		{
			list.push_back(names ? ModPackageProject::OperatorToString(value) : ModPackageProject::OperatorToSymbolicName(value));
		}
		return list;
	}
}

namespace Kortex
{
	wxString ModPackageProject::OperatorToSymbolicName(PackageProject::Operator operatorType)
	{
		using namespace PackageProject;

		switch (operatorType)
		{
			case Operator::Equal:
			{
				return OperatorConst::EQ;
			}
			case Operator::NotEqual:
			{
				return OperatorConst::NOT_EQ;
			}
			case Operator::LessThan:
			{
				return OperatorConst::LT;
			}
			case Operator::LessThanOrEqual:
			{
				return OperatorConst::LTEQ;
			}
			case Operator::GreaterThan:
			{
				return OperatorConst::GT;
			}
			case Operator::GreaterThanOrEqual:
			{
				return OperatorConst::GTEQ;
			}
			case Operator::And:
			{
				return OperatorConst::AND;
			}
			case Operator::Or:
			{
				return OperatorConst::OR;
			}
		};
		return OperatorConst::NONE;
	}
	wxString ModPackageProject::OperatorToString(PackageProject::Operator operatorType)
	{
		using namespace PackageProject;

		switch (operatorType)
		{
			case Operator::Equal:
			{
				return OperatorConst::EQ_STRING;
			}
			case Operator::NotEqual:
			{
				return OperatorConst::NOT_EQ_STRING;
			}
			case Operator::LessThan:
			{
				return OperatorConst::LT_STRING;
			}
			case Operator::LessThanOrEqual:
			{
				return OperatorConst::LTEQ_STRING;
			}
			case Operator::GreaterThan:
			{
				return OperatorConst::GT_STRING;
			}
			case Operator::GreaterThanOrEqual:
			{
				return OperatorConst::GTEQ_STRING;
			}
			case Operator::And:
			{
				return OperatorConst::AND_STRING;
			}
			case Operator::Or:
			{
				return OperatorConst::OR_STRING;
			}
		};
		return OperatorConst::NONE_STRING;
	}
	PackageProject::Operator ModPackageProject::StringToOperator(const wxString& name, bool allowNone, PackageProject::Operator default)
	{
		using namespace PackageProject;

		if (name == OperatorConst::EQ_STRING)
		{
			return Operator::Equal;
		}
		if (name == OperatorConst::NOT_EQ_STRING)
		{
			return Operator::NotEqual;
		}
		if (name == OperatorConst::LT_STRING)
		{
			return Operator::LessThan;
		}
		if (name == OperatorConst::LTEQ_STRING)
		{
			return Operator::LessThanOrEqual;
		}
		if (name == OperatorConst::GT_STRING)
		{
			return Operator::GreaterThan;
		}
		if (name == OperatorConst::GTEQ_STRING)
		{
			return Operator::GreaterThanOrEqual;
		}
		if (name == OperatorConst::AND_STRING)
		{
			return Operator::And;
		}
		if (name == OperatorConst::OR_STRING)
		{
			return Operator::Or;
		}
		if (allowNone && name == OperatorConst::NONE_STRING)
		{
			return Operator::None;
		}
		return default;
	}
	
	KxStringVector ModPackageProject::CreateOperatorSymNamesList(const std::initializer_list<PackageProject::Operator>& operators)
	{
		return CreateOperatorList(operators, false);
	}
	KxStringVector ModPackageProject::CreateOperatorNamesList(const std::initializer_list<PackageProject::Operator>& operators)
	{
		return CreateOperatorList(operators, true);
	}
	
	ModPackageProject::ModPackageProject()
		:m_Config(*this),
		m_Info(*this),
		m_FileData(*this),
		m_Interface(*this),
		m_Requirements(*this),
		m_Components(*this),
	
		m_FormatVersion(ModPackagesModule::GetInstance()->GetModuleInfo().GetVersion()),
		m_TargetProfileID(IGameInstance::GetActive()->GetGameID())
	{
	}
	ModPackageProject::~ModPackageProject()
	{
	}
	
	void ModPackageProject::SetModID(const wxString& id)
	{
		m_ModID = id;
	}
	wxString ModPackageProject::GetModID() const
	{
		// ID -> Name -> translated name -> package file name
		if (!m_ModID.IsEmpty())
		{
			return m_ModID;
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
				return GetConfig().GetInstallPackageFile().AfterLast('\\').BeforeFirst('.');
			}
		}
		return wxEmptyString;
	}
	wxString ModPackageProject::GetModName() const
	{
		// Name -> translated name -> ID (using 'GetModID')
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
			return GetModID();
		}
		return wxEmptyString;
	}
	wxString ModPackageProject::GetSignature() const
	{
		return Kortex::ModManager::BasicGameMod::GetSignatureFromID(GetModID());
	}
}

namespace Kortex::PackageProject
{
	void ConditionChecker::operator()(bool value, Operator operatorType)
	{
		if (m_IsFirstElement)
		{
			m_Result = value;
			m_IsFirstElement = false;
		}
		else
		{
			if (operatorType == Operator::Or)
			{
				m_Result = value || m_Result;
			}
			else
			{
				m_Result = value && m_Result;
			}
		}
	}
}
