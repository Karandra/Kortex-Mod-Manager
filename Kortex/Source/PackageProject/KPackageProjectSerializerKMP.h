#pragma once
#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include <KxFramework/KxXML.h>
class KPackageProject;

class KPackageProjectSerializerKMP: public KPackageProjectSerializer
{
	private:
		const bool m_AsProject = false;
		KPackageProject* m_ProjectLoad = nullptr;
		const KPackageProject* m_ProjectSave = nullptr;
		wxString m_Data;
		KxXMLDocument m_XML;

	private:
		void ReadBase();
		void ReadConfig();
		void ReadInfo();
		void ReadInterface();
		void ReadFiles();
		void ReadRequirements();
		void ReadComponents();

		KxXMLNode WriteBase();
		void WriteConfig(KxXMLNode& baseNode);
		void WriteInfo(KxXMLNode& baseNode);
		void WriteInterface(KxXMLNode& baseNode);
		void WriteFiles(KxXMLNode& baseNode);
		void WriteRequirements(KxXMLNode& baseNode);
		void WriteComponents(KxXMLNode& baseNode);

	public:
		KPackageProjectSerializerKMP(bool bAsProject = false)
			:m_AsProject(bAsProject)
		{
			SetPackageDataRoot(GetDefaultFOModRoot());
		}

	public:
		virtual void Serialize(const KPackageProject* project) override;
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
