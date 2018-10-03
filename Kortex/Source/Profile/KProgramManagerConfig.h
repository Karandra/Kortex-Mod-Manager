#pragma once
#include "stdafx.h"
#include "ProgramManager/KProgramManager.h"
#include <KxFramework/KxSingleton.h>
class KProfile;

//////////////////////////////////////////////////////////////////////////
class KProgramManagerConfig: public KxSingletonPtr<KProgramManagerConfig>
{
	public:
		enum class ProgramType
		{
			Main,
			PreMain,
			PostMain,
		};

	private:
		KRMProgramEntryArray m_EntriesMain;
		KRMProgramEntryArray m_EntriesPre;
		KRMProgramEntryArray m_EntriesPost;

	public:
		KProgramManagerConfig(KProfile& profile, const KxXMLNode& node);

	public:
		size_t GetEntriesCount(ProgramType type) const;
		const KProgramManagerEntry* GetEntryAt(ProgramType type, size_t i) const;
};
