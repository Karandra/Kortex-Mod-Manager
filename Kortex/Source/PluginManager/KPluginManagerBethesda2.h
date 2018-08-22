#pragma once
#include "stdafx.h"
#include "KPluginManagerBethesda.h"
#include "KPluginEntryBethesda2.h"

class KPluginManagerBethesda2: public KPluginManagerBethesda
{
	protected:
		virtual bool CheckExtension(const wxString& name) const;
		virtual void LoadNativeActiveBG() override;

		virtual wxString OnWriteToLoadOrder(const KPluginEntry& entry) const override;
		virtual wxString OnWriteToActiveOrder(const KPluginEntry& entry) const override;

		intptr_t CountLightActiveBefore(const KPluginEntry& modEntry) const;

	public:
		KPluginManagerBethesda2(const wxString& interfaceName, const KxXMLNode& configNode);
		virtual ~KPluginManagerBethesda2();

	public:
		virtual KPluginEntryBethesda2* NewPluginEntry(const wxString& name, bool isActive) const override;

		virtual intptr_t GetPluginDisplayPriority(const KPluginEntry& modEntry) const;
		virtual wxString FormatPriority(const KPluginEntry& modEntry, intptr_t value) const override;
};
