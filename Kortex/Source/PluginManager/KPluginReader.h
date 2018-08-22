#pragma once
#include "stdafx.h"
enum KPMPluginEntryType;

class KPMPluginReader
{
	private:
		bool m_IsRead = false;
		wxString m_FullPath;

	protected:
		virtual void DoReadData() = 0;
		virtual KPMPluginEntryType DoGetFormat() const = 0;
		virtual KxStringVector DoGetDependencies() const = 0;
		virtual wxString DoGetAuthor() const = 0;
		virtual wxString DoGetDescription() const = 0;

		const wxString& GetFullPath() const
		{
			return m_FullPath;
		}

	public:
		KPMPluginReader()
		{
		}
		virtual ~KPMPluginReader()
		{
		}

		virtual void Create(const wxString& fullPath);

	public:
		bool IsOK() const;
		bool IsMaster() const;

		KPMPluginEntryType GetFormat()
		{
			if (!m_IsRead)
			{
				DoReadData();
				m_IsRead = true;
			}
			return DoGetFormat();
		}
		KxStringVector GetDependencies()
		{
			if (!m_IsRead)
			{
				DoReadData();
				m_IsRead = true;
			}
			return DoGetDependencies();
		}
		wxString GetAuthor()
		{
			if (!m_IsRead)
			{
				DoReadData();
				m_IsRead = true;
			}
			return DoGetAuthor();
		}
		wxString GetDescription()
		{
			if (!m_IsRead)
			{
				DoReadData();
				m_IsRead = true;
			}
			return DoGetDescription();
		}
};
