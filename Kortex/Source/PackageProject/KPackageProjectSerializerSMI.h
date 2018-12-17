#pragma once
#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxVersion.h>
class KPackageProject;
class KPPRRequirementEntry;

class KPackageProjectSerializerSMI: public KPackageProjectSerializer
{
	private:
		KxVersion m_ProjectVersion;
		KPackageProject* m_Project = nullptr;
		wxString m_Data;
		KxXMLDocument m_XML;

	private:
		wxString ConvertMultiLine(const wxString& source) const;
		wxString ConvertVariable(const wxString& sOldVariable) const;
		void AddSite(const wxString& url);
		void FixRequirementID(KPPRRequirementEntry* entry) const;
		bool IsComponentsUsed() const;
		void ReadInterface3x4x5x(const wxString& sLogoNodeName);
		void ReadFiles3x4x();

	private:
		KxVersion ReadBase();
		void ReadConfig();

		// 3.0+
		void ReadInfo3x();
		void ReadFiles3x();
		void ReadInterface3x();
		void ReadRequirements3x();
		void ReadComponents3x();

		// 4.0+
		void ReadInfo4x();
		void ReadFiles4x();
		void ReadInterface4x();
		void ReadRequirements4x();
		void ReadComponents4x();

		// 5.0+
		void ReadInfo5x();
		void ReadInterface5x();
		void ReadFiles5x();
		void ReadRequirements5x();
		void ReadComponents5x();
		void ReadINI5x();

	public:
		virtual void Serialize(const KPackageProject* project) override
		{
		}
		virtual void Structurize(KPackageProject* project) override;

		const wxString& GetData() const
		{
			return m_Data;
		}
		void SetData(const wxString& sData)
		{
			m_Data = sData;
		}
};
