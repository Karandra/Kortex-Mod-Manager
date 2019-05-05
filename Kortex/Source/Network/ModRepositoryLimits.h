#pragma once
#include "stdafx.h"

namespace Kortex::NetworkManager
{
	struct ModRepositoryLimitsData
	{
		public:
			int HourlyTotal = -1;
			int HourlyRemaining = -1;
			wxDateTime HourlyLimitReset;

			int DailyTotal = -1;
			int DailyRemaining = -1;
			wxDateTime DailyLimitReset;

		public:
			ModRepositoryLimitsData() = default;
			ModRepositoryLimitsData(int hourlyTotal,
									int hourlyRemaining,
									const wxDateTime& hourlyLimitReset,
									int dailyTotal,
									int dailyRemaining,
									const wxDateTime& dailyLimitReset
			)
				:HourlyTotal(hourlyTotal), HourlyRemaining(hourlyRemaining), HourlyLimitReset(hourlyLimitReset),
				DailyTotal(dailyTotal), DailyRemaining(dailyRemaining), DailyLimitReset(dailyLimitReset)
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

		private:
			double GetRatio(double value, double total) const
			{
				if (total > 0)
				{
					return value / total;
				}
				return 0;
			}

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
				return HasHourlyLimit() || HasDailyLimit();
			}
			bool AnyLimitDepleted() const
			{
				return HourlyDepleted() || DailyDepleted();
			}

		public:
			// Hourly
			bool HasHourlyLimit() const
			{
				return m_Data->HourlyTotal >= 0;
			}
			bool HourlyDepleted() const
			{
				return m_Data->HourlyRemaining == 0;
			}

			int GetHourlyTotal() const
			{
				return m_Data->HourlyTotal;
			}
			int GetHourlyRemaining() const
			{
				return m_Data->HourlyRemaining;
			}
			double GetHourlyRemainingRatio() const
			{
				return GetRatio(m_Data->HourlyRemaining, m_Data->HourlyTotal);
			}
			wxDateTime GetHourlyReset() const
			{
				return m_Data->HourlyLimitReset;
			}

		public:
			// Daily
			bool HasDailyLimit() const
			{
				return m_Data->DailyTotal >= 0;
			}
			bool DailyDepleted() const
			{
				return m_Data->DailyRemaining == 0;
			}
			
			int GetDailyTotal() const
			{
				return m_Data->DailyTotal;
			}
			int GetDailyRemaining() const
			{
				return m_Data->DailyRemaining;
			}
			double GetDailyRemainingRatio() const
			{
				return GetRatio(m_Data->DailyRemaining, m_Data->DailyTotal);
			}
			wxDateTime GetDailyReset() const
			{
				return m_Data->DailyLimitReset;
			}
	};
}
