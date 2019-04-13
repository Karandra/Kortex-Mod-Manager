#pragma once
#include "stdafx.h"

namespace Kortex::NetworkManager
{
	struct ModRepositoryLimitsData
	{
		public:
			int HourlyLimit = -1;
			int HourlyRemaining = -1;
			wxDateTime HourlyLimitReset;

			int DailyLimit = -1;
			int DailyRemaining = -1;
			wxDateTime DailyLimitReset;

		public:
			ModRepositoryLimitsData() = default;
			ModRepositoryLimitsData(int hourlyLimit,
									int hourlyRemaining,
									const wxDateTime& hourlyLimitReset,
									int dailyLimit,
									int dailyRemaining,
									const wxDateTime& dailyLimitReset
			)
				:HourlyLimit(hourlyLimit), HourlyRemaining(hourlyRemaining), HourlyLimitReset(hourlyLimitReset),
				DailyLimit(dailyLimit), DailyRemaining(dailyRemaining), DailyLimitReset(dailyLimitReset)
			{
			}
	};
}

namespace Kortex
{
	class ModRepositoryLimits
	{
		public:
			using Data = NetworkManager::ModRepositoryLimitsData;

		private:
			std::optional<Data> m_Data;

		public:
			ModRepositoryLimits() = default;
			ModRepositoryLimits(const Data& data)
				:m_Data(data)
			{
			}
			
		public:
			bool IsOK() const
			{
				return m_Data.has_value();
			}
			bool HasLimits() const
			{
				return !IsOK() || (HasHourlyLimit() || HasDailyLimit());
			}

			bool HasHourlyLimit() const
			{
				return m_Data->HourlyLimit >= 0;
			}
			int GetHourlyLimit() const
			{
				return m_Data->HourlyLimit;
			}
			int GetHourlyRemaining() const
			{
				return m_Data->HourlyRemaining;
			}
			wxDateTime GetHourlyReset() const
			{
				return m_Data->HourlyLimitReset;
			}

			bool HasDailyLimit() const
			{
				return m_Data->DailyLimit >= 0;
			}
			int GetDailyLimit() const
			{
				return m_Data->DailyLimit;
			}
			int GetDailyRemaining() const
			{
				return m_Data->DailyRemaining;
			}
			wxDateTime GetDailyReset() const
			{
				return m_Data->DailyLimitReset;
			}
	};
}
