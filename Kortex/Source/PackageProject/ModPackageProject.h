#pragma once
#include "stdafx.h"
#include "ConfigSection.h"
#include "InfoSection.h"
#include "FileDataSection.h"
#include "InterfaceSection.h"
#include "RequirementsSection.h"
#include "ComponentsSection.h"
#include "Common.h"
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class ModPackageProject
	{
		public:
			static wxString OperatorToSymbolicName(PackageProject::Operator operatorType);
			static wxString OperatorToString(PackageProject::Operator operatorType);
			static PackageProject::Operator StringToOperator(const wxString& name, bool allowNone, PackageProject::Operator default);
	
			static KxStringVector CreateOperatorSymNamesList(const std::initializer_list<PackageProject::Operator>& operators);
			static KxStringVector CreateOperatorNamesList(const std::initializer_list<PackageProject::Operator>& operators);
	
		private:
			KxVersion m_FormatVersion;
			wxString m_TargetProfileID;
			wxString m_ModID;
	
			// Project parts
			PackageProject::ConfigSection m_Config;
			PackageProject::InfoSection m_Info;
			PackageProject::FileDataSection m_FileData;
			PackageProject::InterfaceSection m_Interface;
			PackageProject::RequirementsSection m_Requirements;
			PackageProject::ComponentsSection m_Components;
	
		public:
			ModPackageProject();
			virtual ~ModPackageProject();
	
		public:
			const KxVersion& GetFormatVersion() const
			{
				return m_FormatVersion;
			}
			void SetFormatVersion(const KxVersion& id)
			{
				m_FormatVersion = id;
			}
			
			const wxString& GetTargetProfileID() const
			{
				return m_TargetProfileID;
			}
			void SetTargetProfileID(const wxString& id)
			{
				m_TargetProfileID = id;
			}
			
			void SetModID(const wxString& id);
			wxString GetModID() const;
			wxString GetModName() const;
			wxString GetSignature() const;
	
			PackageProject::ConfigSection& GetConfig()
			{
				return m_Config;
			}
			const PackageProject::ConfigSection& GetConfig() const
			{
				return m_Config;
			}
			
			PackageProject::InfoSection& GetInfo()
			{
				return m_Info;
			}
			const PackageProject::InfoSection& GetInfo() const
			{
				return m_Info;
			}
			
			PackageProject::FileDataSection& GetFileData()
			{
				return m_FileData;
			}
			const PackageProject::FileDataSection& GetFileData() const
			{
				return m_FileData;
			}
			
			PackageProject::InterfaceSection& GetInterface()
			{
				return m_Interface;
			}
			const PackageProject::InterfaceSection& GetInterface() const
			{
				return m_Interface;
			}
			
			PackageProject::RequirementsSection& GetRequirements()
			{
				return m_Requirements;
			}
			const PackageProject::RequirementsSection& GetRequirements() const
			{
				return m_Requirements;
			}
			
			PackageProject::ComponentsSection& GetComponents()
			{
				return m_Components;
			}
			const PackageProject::ComponentsSection& GetComponents() const
			{
				return m_Components;
			}
	};
}

namespace Kortex::PackageProject
{
	class ConditionChecker final
	{
		private:
			bool m_Result = false;
			bool m_IsFirstElement = true;

		public:
			void operator()(bool value, Operator operatorType);

		public:
			bool GetResult() const
			{
				return m_Result;
			}
	};
}
