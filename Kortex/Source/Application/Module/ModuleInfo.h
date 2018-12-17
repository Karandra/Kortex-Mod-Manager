#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
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
			virtual KImageEnum GetImageID() const = 0;
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
			KImageEnum m_ImageID;

		public:
			SimpleModuleInfo(const wxString& id, const wxString& name, const KxVersion& version, KImageEnum imageID)
				:m_ID(id), m_Name(name), m_Version(version), m_ImageID(imageID)
			{
			}

		public:
			virtual wxString GetID() const override
			{
				return m_ID;
			}
			virtual wxString GetName() const override
			{
				return KTr(m_Name);
			}
			virtual KxVersion GetVersion() const override
			{
				return m_Version;
			}
			virtual KImageEnum GetImageID() const override
			{
				return m_ImageID;
			}
	};
}
