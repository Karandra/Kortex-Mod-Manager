#pragma once
#include "stdafx.h"
#include "PackageProject/Serializer.h"
#include <KxFramework/KxXML.h>

namespace Kortex
{
	class ModPackageProject;
}

namespace Kortex::PackageProject
{
	class NativeSerializer: public Serializer
	{
		private:
			const bool m_AsProject = false;
			ModPackageProject* m_ProjectLoad = nullptr;
			const ModPackageProject* m_ProjectSave = nullptr;
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
			NativeSerializer(bool bAsProject = false)
				:m_AsProject(bAsProject)
			{
				SetPackageDataRoot(GetDefaultFOModRoot());
			}
			
		public:
			void Serialize(const ModPackageProject* project) override;
			void Structurize(ModPackageProject* project) override;
			
			const wxString& GetData() const
			{
				return m_Data;
			}
			void SetData(const wxString& sData)
			{
				m_Data = sData;
			}
	};
}
