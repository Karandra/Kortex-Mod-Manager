#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IManagerInfo
	{
		public:
			virtual ~IManagerInfo() = default;

		public:
			virtual wxString GetID() const = 0;
			virtual wxString GetName() const = 0;
	};
}

namespace Kortex
{
	class SimpleManagerInfo: public IManagerInfo
	{
		private:
			wxString m_ID;
			wxString m_Name;

		public:
			SimpleManagerInfo(const wxString& id, const wxString& name)
				:m_ID(id), m_Name(name)
			{
			}

		public:
			virtual wxString GetID() const override
			{
				return m_ID;
			}
			virtual wxString GetName() const override
			{
				return m_Name;
			}
	};
}
