#pragma once
#include "stdafx.h"
#include "KPluginEntryBethesda.h"
class KPluginReaderBethesda;

class KPluginEntryBethesda2: public KPluginEntryBethesda
{
	public:
		using Vector = std::vector<std::unique_ptr<KPluginEntryBethesda2>>;
		using RefVector = std::vector<KPluginEntryBethesda2*>;

	protected:
		virtual void OnPluginRead(const KPluginReader& reader) override;

	public:
		KPluginEntryBethesda2(const wxString& name, bool isActive)
			:KPluginEntryBethesda(name, isActive)
		{
		}
};
