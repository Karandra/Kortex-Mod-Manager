#pragma once
#include "stdafx.h"
#include "KPackageProjectConfig.h"
#include "KPackageProjectInfo.h"
#include "KPackageProjectFileData.h"
#include "KPackageProjectInterface.h"
#include "KPackageProjectRequirements.h"
#include "KPackageProjectComponents.h"
#include "KPackageProjectDefs.h"
#include <KxFramework/KxVersion.h>

namespace Kortex::PackageProject
{
	class KPackageProjectConditionChecker final
	{
		private:
			bool m_Result = false;
			bool m_IsFirstElement = true;
	
		public:
			void operator()(bool value, KPPOperator operatorType);
			
		public:
			bool GetResult() const
			{
				return m_Result;
			}
	};
}

namespace Kortex
{
	class KPackageProject
	{
		public:
			static wxString OperatorToSymbolicName(PackageProject::KPPOperator operatorType);
			static wxString OperatorToString(PackageProject::KPPOperator operatorType);
			static PackageProject::KPPOperator StringToOperator(const wxString& name, bool allowNone, PackageProject::KPPOperator default);
	
			static KxStringVector CreateOperatorSymNamesList(const std::initializer_list<PackageProject::KPPOperator>& operators);
			static KxStringVector CreateOperatorNamesList(const std::initializer_list<PackageProject::KPPOperator>& operators);
	
		private:
			KxVersion m_FormatVersion;
			wxString m_TargetProfileID;
			wxString m_ModID;
	
			// Project parts
			PackageProject::KPackageProjectConfig m_Config;
			PackageProject::KPackageProjectInfo m_Info;
			PackageProject::KPackageProjectFileData m_FileData;
			PackageProject::KPackageProjectInterface m_Interface;
			PackageProject::KPackageProjectRequirements m_Requirements;
			PackageProject::KPackageProjectComponents m_Components;
	
		public:
			KPackageProject();
			virtual ~KPackageProject();
	
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
	
			PackageProject::KPackageProjectConfig& GetConfig()
			{
				return m_Config;
			}
			const PackageProject::KPackageProjectConfig& GetConfig() const
			{
				return m_Config;
			}
			
			PackageProject::KPackageProjectInfo& GetInfo()
			{
				return m_Info;
			}
			const PackageProject::KPackageProjectInfo& GetInfo() const
			{
				return m_Info;
			}
			
			PackageProject::KPackageProjectFileData& GetFileData()
			{
				return m_FileData;
			}
			const PackageProject::KPackageProjectFileData& GetFileData() const
			{
				return m_FileData;
			}
			
			PackageProject::KPackageProjectInterface& GetInterface()
			{
				return m_Interface;
			}
			const PackageProject::KPackageProjectInterface& GetInterface() const
			{
				return m_Interface;
			}
			
			PackageProject::KPackageProjectRequirements& GetRequirements()
			{
				return m_Requirements;
			}
			const PackageProject::KPackageProjectRequirements& GetRequirements() const
			{
				return m_Requirements;
			}
			
			PackageProject::KPackageProjectComponents& GetComponents()
			{
				return m_Components;
			}
			const PackageProject::KPackageProjectComponents& GetComponents() const
			{
				return m_Components;
			}
	};
}
