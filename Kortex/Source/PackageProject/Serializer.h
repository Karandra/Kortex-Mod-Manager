#pragma once
#include "stdafx.h"
#include "Common.h"
#include <Kortex/ModManager.hpp>
#include "Network/ModSourceStore.h"

namespace Kortex
{
	class ModPackageProject;
}

namespace Kortex::PackageProject
{
	class Serializer
	{
		public:
			static const wxString& GetDefaultFOModRoot()
			{
				static const wxString root = "FOMod";
				return root;
			}
			static const wxString& GetDefaultKMPRoot()
			{
				static const wxString root = "KortexPackage";
				return root;
			}
			
			static ModSourceItem TryParseWebSite(const wxString& url, wxString* domainNameOut = nullptr);
			static wxString ConvertBBCode(const wxString& bbSource);
	
		private:
			wxString m_PackageDataRoot;
			
		protected:
			wxString PathNameToPackage(const wxString& pathName, ContentType type) const;
			bool CheckTag(const wxString& tagName) const;
			
		public:
			Serializer();
			virtual ~Serializer();
			
		public:
			virtual void Serialize(const ModPackageProject* project) = 0;
			virtual void Structurize(ModPackageProject* project) = 0;
			
			const wxString& GetPackageDataRoot(const wxString& path) const
			{
				return m_PackageDataRoot;
			}
			void SetPackageDataRoot(const wxString& path)
			{
				m_PackageDataRoot = path;
			}
	};
}
