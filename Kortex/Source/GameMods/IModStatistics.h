#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex::ModStatistics
{
	enum class StatIndex
	{
		TotalMods,
		ActiveMods,
		InactiveMods,
		FilesCount,
		FoldersCount,
		FilesAndFoldersCount,
		UsedSpace,

		MAX,
		MIN = 0
	};

	class StatInfo
	{
		private:
			size_t m_Index = -1;

		public:
			StatInfo(size_t index)
				:m_Index(index)
			{
			}

		public:
			size_t GetIndex() const
			{
				return m_Index;
			}
	};
}

namespace Kortex
{
	class IModStatistics: public KxSingletonPtr<IModStatistics>
	{
		public:
			virtual size_t GetStatCount() const = 0;
			virtual bool HasStat(const ModStatistics::StatInfo& stat) const = 0;

			virtual wxString GetStatName(const ModStatistics::StatInfo& stat) const = 0;
			virtual wxString GetStatValue(const ModStatistics::StatInfo& stat) const = 0;
	};
}
