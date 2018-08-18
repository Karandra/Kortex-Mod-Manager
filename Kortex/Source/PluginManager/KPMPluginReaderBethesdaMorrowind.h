#pragma once
#include "stdafx.h"
#include "KPMPluginReader.h"

class KPMPluginReaderBethesdaMorrowind: public KPMPluginReader
{
	private:
		KPMPluginEntryType m_Format;
		KxStringVector m_Dependencies;
		wxString m_Author;
		wxString m_Description;

	protected:
		virtual void DoReadData() override;

		virtual KPMPluginEntryType DoGetFormat() const override
		{
			return m_Format;
		}
		virtual KxStringVector DoGetDependencies() const override
		{
			return m_Dependencies;
		}
		virtual wxString DoGetAuthor() const override
		{
			return m_Author;
		}
		virtual wxString DoGetDescription() const override
		{
			return m_Description;
		}
};
