#pragma once
#include "stdafx.h"
#include "KPluginManagerBethesdaGeneric.h"

class KPluginManagerBethesdaGeneric2: public KPluginManagerBethesdaGeneric
{
	protected:
		virtual KPMPluginEntryType GetPluginTypeFromPath(const wxString& name) const override;
		virtual bool IsEntryTypeSupported(KPMPluginEntryType type) const override
		{
			const auto nMask = KPMPE_TYPE_NORMAL|KPMPE_TYPE_MASTER|KPMPE_TYPE_LIGHT;
			return (~nMask & type) == 0;
		}

		virtual void LoadNativeActiveBG() override;

		virtual wxString OnWriteToLoadOrder(const KPMPluginEntry* entry) const override;
		virtual wxString OnWriteToActiveOrder(const KPMPluginEntry* entry) const override;

	public:
		KPluginManagerBethesdaGeneric2(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig);
		virtual ~KPluginManagerBethesdaGeneric2();
};
