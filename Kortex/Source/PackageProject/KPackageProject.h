#pragma once
#include "stdafx.h"
#include "KPackageProjectConfig.h"
#include "KPackageProjectInfo.h"
#include "KPackageProjectFileData.h"
#include "KPackageProjectInterface.h"
#include "KPackageProjectRequirements.h"
#include "KPackageProjectComponents.h"
#include "KPackageProjectDefs.h"
#include "KApp.h"
#include <KxFramework/KxVersion.h>

//////////////////////////////////////////////////////////////////////////
class KPackageProjectConditionChecker
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

//////////////////////////////////////////////////////////////////////////
class KPackageProject
{
	public:
		static wxString OperatorToSymbolicName(KPPOperator operatorType);
		static wxString OperatorToString(KPPOperator operatorType);
		static KPPOperator StringToOperator(const wxString& name, bool allowNone, KPPOperator default);

		static KxStringVector CreateOperatorSymNamesList(const std::initializer_list<KPPOperator>& operators);
		static KxStringVector CreateOperatorNamesList(const std::initializer_list<KPPOperator>& operators);

	private:
		KxVersion m_FormatVersion;
		wxString m_TargetProfileID;
		wxString m_ModID;

		// Project parts
		KPackageProjectConfig m_Config;
		KPackageProjectInfo m_Info;
		KPackageProjectFileData m_FileData;
		KPackageProjectInterface m_Interface;
		KPackageProjectRequirements m_Requirements;
		KPackageProjectComponents m_Components;

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

		KPackageProjectConfig& GetConfig()
		{
			return m_Config;
		}
		const KPackageProjectConfig& GetConfig() const
		{
			return m_Config;
		}
		
		KPackageProjectInfo& GetInfo()
		{
			return m_Info;
		}
		const KPackageProjectInfo& GetInfo() const
		{
			return m_Info;
		}
		
		KPackageProjectFileData& GetFileData()
		{
			return m_FileData;
		}
		const KPackageProjectFileData& GetFileData() const
		{
			return m_FileData;
		}
		
		KPackageProjectInterface& GetInterface()
		{
			return m_Interface;
		}
		const KPackageProjectInterface& GetInterface() const
		{
			return m_Interface;
		}
		
		KPackageProjectRequirements& GetRequirements()
		{
			return m_Requirements;
		}
		const KPackageProjectRequirements& GetRequirements() const
		{
			return m_Requirements;
		}
		
		KPackageProjectComponents& GetComponents()
		{
			return m_Components;
		}
		const KPackageProjectComponents& GetComponents() const
		{
			return m_Components;
		}
};
