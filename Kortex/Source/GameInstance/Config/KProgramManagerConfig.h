#pragma once
#include "stdafx.h"
#include "ProgramManager/KProgramManager.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;

//////////////////////////////////////////////////////////////////////////
class KProgramManagerConfig: public KxSingletonPtr<KProgramManagerConfig>
{
	private:
		KProgramEntry::Vector m_Entries;

	public:
		KProgramManagerConfig(KGameInstance& profile, const KxXMLNode& rootNode);

	public:
		size_t GetProgramsCount() const
		{
			return m_Entries.size();
		}
		const KProgramEntry::Vector& GetPrograms() const
		{
			return m_Entries;
		}
};
