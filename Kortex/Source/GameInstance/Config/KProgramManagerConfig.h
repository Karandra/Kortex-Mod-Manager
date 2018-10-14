#pragma once
#include "stdafx.h"
#include "ProgramManager/KProgramManager.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;

//////////////////////////////////////////////////////////////////////////
class KProgramManagerConfig: public KxSingletonPtr<KProgramManagerConfig>
{
	private:
		KProgramManagerEntry::Vector m_Entries;

	public:
		KProgramManagerConfig(KGameInstance& profile, const KxXMLNode& rootNode);

	public:
		size_t GetProgramsCount() const
		{
			return m_Entries.size();
		}
		const KProgramManagerEntry::Vector& GetPrograms() const
		{
			return m_Entries;
		}
};
