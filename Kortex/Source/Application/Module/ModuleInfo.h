#pragma once
#include "stdafx.h"
#include "Application/Resources/ResourceID.h"
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class IModuleInfo
	{
		public:
			virtual ~IModuleInfo() = default;

		public:
			virtual wxString GetID() const = 0;
			virtual wxString GetName() const = 0;
			virtual KxVersion GetVersion() const = 0;
			virtual ResourceID GetImageID() const = 0;
	};
}

namespace Kortex
{
	class SimpleModuleInfo: public IModuleInfo
	{
		private:
			wxString m_ID;
			wxString m_Name;
			KxVersion m_Version;
			ResourceID m_ImageID;

		public:
			SimpleModuleInfo(const wxString& id, const wxString& name, const KxVersion& version, const ResourceID& imageID)
				:m_ID(id), m_Name(name), m_Version(version), m_ImageID(imageID)
			{
			}

		public:
			virtual wxString GetID() const override
			{
				return m_ID;
			}
			virtual wxString GetName() const override;
			virtual KxVersion GetVersion() const override
			{
				return m_Version;
			}
			virtual ResourceID GetImageID() const override
			{
				return m_ImageID;
			}
	};
}
