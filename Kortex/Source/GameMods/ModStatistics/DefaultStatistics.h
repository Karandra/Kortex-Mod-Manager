#pragma once
#include "stdafx.h"
#include "GameMods/IModStatistics.h"

namespace Kortex::ModStatistics
{
	class DefaultStatistics: public IModStatistics
	{
		protected:
			int64_t GetStatValueInt(const StatInfo& stat) const;

		public:
			size_t GetStatCount() const override;
			bool HasStat(const StatInfo& stat) const override;

			wxString GetStatName(const StatInfo& stat) const override;
			wxString GetStatValue(const StatInfo& stat) const override;
	};
}
