#pragma once
#include "stdafx.h"
#include "KRTTI.h"
class KPluginEntry;

class KPluginReader: public KRTTI::CastAsIs<KPluginReader>
{
	friend class KPluginEntry;

	protected:
		virtual bool IsOK() const = 0;
		virtual void DoReadData(const KPluginEntry& pluginEntry) = 0;

	public:
		virtual ~KPluginReader() = default;
};
