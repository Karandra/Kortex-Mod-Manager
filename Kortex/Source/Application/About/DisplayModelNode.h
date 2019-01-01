#pragma once
#include "stdafx.h"
#include "Utility/KImageProvider.h"
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class IModule;
}

namespace Kortex::Application::About
{
	class INode
	{
		protected:
			class LicenseData
			{
				friend class INode;

				private:
					wxString m_License;
					bool m_ShouldLoad = true;

				public:
					operator const wxString&() const
					{
						return m_License;
					}
			};

		public:
			using Vector = std::vector<std::unique_ptr<INode>>;

		public:
			enum class Type
			{
				Application,
				Software,
				Resource,
			};

		protected:
			wxString GetLocation(Type type) const;
			wxString ReadLicense(Type type) const;
			const wxString& LoadLicense(LicenseData& data, Type type) const;

		public:
			virtual ~INode() = default;

		public:
			virtual wxString GetName() const = 0;
			virtual KxVersion GetVersion() const = 0;
			virtual KImageEnum GetIconID() const = 0;
			virtual wxString GetLicense() const = 0;
			virtual wxString GetURL() const = 0;
	};
}

namespace Kortex::Application::About
{
	class AppNode: public INode
	{
		private:
			mutable LicenseData m_Licence;

		public:
			wxString GetName() const override;
			KxVersion GetVersion() const override;
			KImageEnum GetIconID() const override;
			wxString GetLicense() const override;
			wxString GetURL() const override;
	};
}

namespace Kortex::Application::About
{
	class ModuleNode: public INode
	{
		private:
			const IModule& m_Module;
			mutable LicenseData m_Licence;

		public:
			ModuleNode(const IModule& module)
				:m_Module(module)
			{
			}

		public:
			wxString GetName() const override;
			KxVersion GetVersion() const override;
			KImageEnum GetIconID() const override;
			wxString GetLicense() const override;
			wxString GetURL() const override
			{
				return wxString();
			}
	};
}

namespace Kortex::Application::About
{
	class GenericNode: public INode
	{
		private:
			const Type m_Type;
			const wxString m_Name;
			const KxVersion m_Version;
			const wxString m_URL;
			const KImageEnum m_IconID;
			mutable LicenseData m_Licence;

		public:
			GenericNode(Type type, const wxString& name, const KxVersion& version, const wxString& url, KImageEnum iconID)
				:m_Type(type), m_Name(name), m_Version(version), m_URL(url), m_IconID(iconID)
			{
			}

		public:
			wxString GetName() const override
			{
				return m_Name;
			}
			KxVersion GetVersion() const override
			{
				return m_Version;
			}
			KImageEnum GetIconID() const override
			{
				return m_IconID;
			}
			wxString GetLicense() const override
			{
				return LoadLicense(m_Licence, m_Type);
			}
			wxString GetURL() const override
			{
				return m_URL;
			}
	};
}

namespace Kortex::Application::About
{
	class SoftwareNode: public GenericNode
	{
		public:
			SoftwareNode(const wxString& name, const KxVersion& version, const wxString& url, KImageEnum iconID = KIMG_NONE)
				:GenericNode(Type::Software, name, version, url, iconID)
			{
			}
	};
}

namespace Kortex::Application::About
{
	class ResourceNode: public INode
	{
		private:
			const wxString m_Name;
			const wxString m_URL;
			mutable LicenseData m_Licence;

		public:
			ResourceNode(const wxString& name, const wxString& url)
				:m_Name(name), m_URL(url)
			{
			}

		public:
			wxString GetName() const override
			{
				return m_Name;
			}
			KxVersion GetVersion() const override
			{
				return KxVersion();
			}
			KImageEnum GetIconID() const override
			{
				return KIMG_NONE;
			}
			wxString GetLicense() const override
			{
				return LoadLicense(m_Licence, Type::Resource);
			}
			wxString GetURL() const override
			{
				return m_URL;
			}
	};
}
