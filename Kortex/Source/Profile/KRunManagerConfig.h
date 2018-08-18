#pragma once
#include "stdafx.h"
#include "RunManager/KRunManager.h"
#include <KxFramework/KxSingleton.h>
class KProfile;

enum KPRCEType
{
	KPRCE_TYPE_MAIN,
	KPRCE_TYPE_PREMAIN,
	KPRCE_TYPE_POSTMAIN,
};

//////////////////////////////////////////////////////////////////////////
class KRunManagerConfig: public KxSingletonPtr<KRunManagerConfig>
{
	private:
		KRMProgramEntryArray m_EntriesMain;
		KRMProgramEntryArray m_EntriesPre;
		KRMProgramEntryArray m_EntriesPost;

	public:
		KRunManagerConfig(KProfile& profile, KxXMLNode& node);

	public:
		size_t GetEntriesCount(KPRCEType type) const;
		const KRunManagerProgram* GetEntryAt(KPRCEType type, size_t i) const;
};
