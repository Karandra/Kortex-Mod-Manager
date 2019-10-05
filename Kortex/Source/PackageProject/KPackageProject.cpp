#include "stdafx.h"
#include "KPackageProject.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/PackageManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

namespace 
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
	namespace Util
	{
		KxStringVector CreateOperatorList(const std::initializer_list<Kortex::PackageDesigner::KPPOperator>& operators, bool names)
		{
			using namespace Kortex::PackageDesigner;

			KxStringVector list;
			list.reserve(operators.size());
	
			for (KPPOperator value: operators)
			{
				list.push_back(names ? KPackageProject::OperatorToString(value) : KPackageProject::OperatorToSymbolicName(value));
			}
			return list;
		}
	}
}

namespace Kortex::PackageDesigner
{
	void KPackageProjectConditionChecker::operator()(bool value, KPPOperator operatorType)
	{
		if (m_IsFirstElement)
		{
			m_Result = value;
			m_IsFirstElement = false;
		}
		else
		{
			if (operatorType == KPP_OPERATOR_OR)
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

namespace Kortex::PackageDesigner
{
	wxString KPackageProject::OperatorToSymbolicName(KPPOperator operatorType)
	{
		switch (operatorType)
		{
			case KPP_OPERATOR_EQ:
			{
				return OperatorConst::EQ;
			}
			case KPP_OPERATOR_NOT_EQ:
			{
				return OperatorConst::NOT_EQ;
			}
			case KPP_OPERATOR_LT:
			{
				return OperatorConst::LT;
			}
			case KPP_OPERATOR_LTEQ:
			{
				return OperatorConst::LTEQ;
			}
			case KPP_OPERATOR_GT:
			{
				return OperatorConst::GT;
			}
			case KPP_OPERATOR_GTEQ:
			{
				return OperatorConst::GTEQ;
			}
			case KPP_OPERATOR_AND:
			{
				return OperatorConst::AND;
			}
			case KPP_OPERATOR_OR:
			{
				return OperatorConst::OR;
			}
		};
		return OperatorConst::NONE;
	}
	wxString KPackageProject::OperatorToString(KPPOperator operatorType)
	{
		switch (operatorType)
		{
			case KPP_OPERATOR_EQ:
			{
				return OperatorConst::EQ_STRING;
			}
			case KPP_OPERATOR_NOT_EQ:
			{
				return OperatorConst::NOT_EQ_STRING;
			}
			case KPP_OPERATOR_LT:
			{
				return OperatorConst::LT_STRING;
			}
			case KPP_OPERATOR_LTEQ:
			{
				return OperatorConst::LTEQ_STRING;
			}
			case KPP_OPERATOR_GT:
			{
				return OperatorConst::GT_STRING;
			}
			case KPP_OPERATOR_GTEQ:
			{
				return OperatorConst::GTEQ_STRING;
			}
			case KPP_OPERATOR_AND:
			{
				return OperatorConst::AND_STRING;
			}
			case KPP_OPERATOR_OR:
			{
				return OperatorConst::OR_STRING;
			}
		};
		return OperatorConst::NONE_STRING;
	}
	KPPOperator KPackageProject::StringToOperator(const wxString& name, bool allowNone, KPPOperator default)
	{
		if (name == OperatorConst::EQ_STRING)
		{
			return KPP_OPERATOR_EQ;
		}
		if (name == OperatorConst::NOT_EQ_STRING)
		{
			return KPP_OPERATOR_NOT_EQ;
		}
		if (name == OperatorConst::LT_STRING)
		{
			return KPP_OPERATOR_LT;
		}
		if (name == OperatorConst::LTEQ_STRING)
		{
			return KPP_OPERATOR_LTEQ;
		}
		if (name == OperatorConst::GT_STRING)
		{
			return KPP_OPERATOR_GT;
		}
		if (name == OperatorConst::GTEQ_STRING)
		{
			return KPP_OPERATOR_GTEQ;
		}
		if (name == OperatorConst::AND_STRING)
		{
			return KPP_OPERATOR_AND;
		}
		if (name == OperatorConst::OR_STRING)
		{
			return KPP_OPERATOR_OR;
		}
		if (allowNone && name == OperatorConst::NONE_STRING)
		{
			return KPP_OPERATOR_NONE;
		}
		return default;
	}
	
	KxStringVector KPackageProject::CreateOperatorSymNamesList(const std::initializer_list<KPPOperator>& operators)
	{
		return Util::CreateOperatorList(operators, false);
	}
	KxStringVector KPackageProject::CreateOperatorNamesList(const std::initializer_list<KPPOperator>& operators)
	{
		return Util::CreateOperatorList(operators, true);
	}
	
	KPackageProject::KPackageProject()
		:m_Config(*this),
		m_Info(*this),
		m_FileData(*this),
		m_Interface(*this),
		m_Requirements(*this),
		m_Components(*this),
	
		m_FormatVersion(Kortex::ModPackagesModule::GetInstance()->GetModuleInfo().GetVersion()),
		m_TargetProfileID(Kortex::IGameInstance::GetActive()->GetGameID())
	{
	}
	KPackageProject::~KPackageProject()
	{
	}
	
	void KPackageProject::SetModID(const wxString& id)
	{
		m_ModID = id;
	}
	wxString KPackageProject::GetModID() const
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
	wxString KPackageProject::GetModName() const
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
	wxString KPackageProject::GetSignature() const
	{
		return Kortex::ModManager::BasicGameMod::GetSignatureFromID(GetModID());
	}
}
