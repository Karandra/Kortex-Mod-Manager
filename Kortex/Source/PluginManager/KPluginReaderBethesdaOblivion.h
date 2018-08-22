#pragma once
#include "stdafx.h"
#include "KPluginReaderBethesda.h"

class KPluginReaderBethesdaOblivion: public KPluginReaderBethesda
{
	private:
		bool m_IsOK = false;

	protected:
		virtual bool IsOK() const override
		{
			return m_IsOK;
		}
		virtual void DoReadData(const KPluginEntry& pluginEntry) override;
};
