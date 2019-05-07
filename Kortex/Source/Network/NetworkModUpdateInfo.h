#pragma once
#include "Common.h"
#include <KxFramework/KxIndexedEnum.h>
#include <KxFramework/KxVersion.h>
class KxXMLNode;

namespace Kortex::NetworkManager
{
	enum class ModUpdateState
	{
		Unknown = 0,
		NoUpdates,

		ModUpdated,
		ModDeleted,

		ModFileUpdated,
		ModFileDeleted
	};
	struct ModUpdateStateDef: KxIndexedEnum::Definition<ModUpdateStateDef, ModUpdateState, wxString, true>
	{
		using UpdateState = ModUpdateState;

		inline static const TItem ms_Index[] =
		{
			{UpdateState::Unknown, wxS("Unknown")},
			{UpdateState::NoUpdates, wxS("NoUpdates")},

			{UpdateState::ModUpdated, wxS("ModUpdated")},
			{UpdateState::ModDeleted, wxS("ModDeleted")},

			{UpdateState::ModFileUpdated, wxS("ModFileUpdated")},
			{UpdateState::ModFileDeleted, wxS("ModFileDeleted")},
		};
	};
}

namespace Kortex::NetworkManager
{
	enum class ModUpdateDetails
	{
		None = 0,
		MarkedOld = 1 << 0,
	};
	inline constexpr ModUpdateDetails operator|(ModUpdateDetails left, ModUpdateDetails right)
	{
		using T = std::underlying_type_t<ModUpdateDetails>;
		return static_cast<ModUpdateDetails>(static_cast<T>(left) | static_cast<T>(right));
	}
	inline constexpr bool operator&(ModUpdateDetails left, ModUpdateDetails right)
	{
		using T = std::underlying_type_t<ModUpdateDetails>;
		return (static_cast<T>(left) & static_cast<T>(right)) != 0;
	}

	struct ModUpdateDetailsDef: KxIndexedEnum::Definition<ModUpdateDetailsDef, ModUpdateDetails, wxString, true>
	{
		using UpdateDetails = ModUpdateDetails;

		inline static const TItem ms_Index[] =
		{
			{UpdateDetails::None, wxS("None")},
			{UpdateDetails::MarkedOld, wxS("MarkedOld")},
		};
	};
}

namespace Kortex
{
	class NetworkModUpdateInfo
	{
		public:
			using UpdateState = NetworkManager::ModUpdateState;
			using UpdateStateDef = NetworkManager::ModUpdateStateDef;

			using UpdateDetails = NetworkManager::ModUpdateDetails;
			using UpdateDetailsDef = NetworkManager::ModUpdateDetailsDef;
			
		private:
			KxVersion m_Version;
			wxDateTime m_UpdateCheckDate;
			UpdateState m_State = UpdateState::Unknown;
			UpdateDetails m_Details = UpdateDetails::None;
			size_t m_ActivityHash = 0;

		public:
			NetworkModUpdateInfo() = default;
			NetworkModUpdateInfo(UpdateState state, const wxDateTime& date)
				:m_State(state), m_UpdateCheckDate(date)
			{
			}

		public:
			bool IsOK() const
			{
				return m_State != UpdateState::NoUpdates && m_UpdateCheckDate.IsValid();
			}

			wxDateTime GetUpdateCheckDate() const
			{
				return m_UpdateCheckDate;
			}
			void SetUpdateCheckDate(const wxDateTime& date)
			{
				m_UpdateCheckDate = date;
			}
			
			bool AnyUpdated() const
			{
				return m_State == UpdateState::ModUpdated || m_State == UpdateState::ModFileUpdated || m_Details & UpdateDetails::MarkedOld;
			}
			bool AnyDeleted() const
			{
				return m_State == UpdateState::ModDeleted || m_State == UpdateState::ModFileDeleted;
			}
			UpdateState GetState() const
			{
				return m_State;
			}
			void SetState(UpdateState state)
			{
				m_State = state;
			}
			
			UpdateDetails GetDetails() const
			{
				return m_Details;
			}
			void SetDetails(UpdateDetails details)
			{
				m_Details = details;
			}
			void ModDetails(UpdateDetails details)
			{
				m_Details = m_Details|details;
			}

			KxVersion GetVersion() const
			{
				return m_Version;
			}
			void SetVersion(const KxVersion& version)
			{
				m_Version = version;
			}

			size_t GetActivityHash() const
			{
				return m_ActivityHash;
			}
			void SetActivityHash(size_t value)
			{
				m_ActivityHash = value;
			}

		public:
			void Save(KxXMLNode& node) const;
			void Load(const KxXMLNode& node);
	};
}
