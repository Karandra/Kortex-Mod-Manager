#pragma once
#include "stdafx.h"
#include "Application/Resources/ImageResourceID.h"
#include <KxFramework/KxVersion.h>
#include <KxFramework/KxURI.h>

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
			virtual ResourceID GetIconID() const = 0;
			virtual KxURI GetURI() const = 0;

			virtual bool HasLicense() const = 0;
			virtual wxString GetLicense() const = 0;
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
			ResourceID GetIconID() const override;
			KxURI GetURI() const override;

			bool HasLicense() const override;
			wxString GetLicense() const override;

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
			ResourceID GetIconID() const override;
			KxURI GetURI() const override
			{
				return {};
			}
			
			bool HasLicense() const override;
			wxString GetLicense() const override;
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
			const KxURI m_URL;
			const ResourceID m_IconID;
			mutable LicenseData m_Licence;

		public:
			GenericNode(Type type, const wxString& name, const KxVersion& version, const wxString& url, const ResourceID& iconID)
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
			ResourceID GetIconID() const override
			{
				return m_IconID;
			}
			KxURI GetURI() const override
			{
				return m_URL;
			}
			
			bool HasLicense() const override
			{
				return !LoadLicense(m_Licence, m_Type).IsEmpty();
			}
			wxString GetLicense() const override
			{
				return LoadLicense(m_Licence, m_Type);
			}
	};
}

namespace Kortex::Application::About
{
	class SoftwareNode: public GenericNode
	{
		public:
			SoftwareNode(const wxString& name, const KxVersion& version, const wxString& url, const ResourceID& iconID = ImageResourceID::None)
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
			const KxURI m_URL;
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
			ResourceID GetIconID() const override
			{
				return {};
			}
			KxURI GetURI() const override
			{
				return m_URL;
			}
			
			bool HasLicense() const override
			{
				return !LoadLicense(m_Licence, Type::Resource).IsEmpty();
			}
			wxString GetLicense() const override
			{
				return LoadLicense(m_Licence, Type::Resource);
			}
	};
}
