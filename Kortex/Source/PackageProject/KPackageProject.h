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
enum KModManagerLocation;

//////////////////////////////////////////////////////////////////////////
class KPackageProject
{
	public:
		// Initialize 'cookie' to zero before first call to this function
		static void CheckCondition(bool& currentStatus, int& cookie, bool isThis, KPPOperator operatorType);
		static void CheckCondition(bool& currentStatus, int& cookie, const KPPRRequirementEntry& entry);
		static void CheckCondition(bool& currentStatus, int& cookie, bool isThis, const KPPCFlagEntry& entry);

	private:
		wxString m_FormatVersion;
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
		const wxString& GetFormatVersion() const
		{
			return m_FormatVersion;
		}
		void SetFormatVersion(const wxString& id)
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
		
		const wxString& GetModID() const
		{
			return m_ModID;
		}
		void SetModID(const wxString& id)
		{
			m_ModID = id;
		}
		wxString ComputeModID() const;
		wxString ComputeModName() const;
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
