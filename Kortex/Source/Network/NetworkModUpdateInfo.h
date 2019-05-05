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

namespace Kortex
{
	class NetworkModUpdateInfo
	{
		public:
			using UpdateState = NetworkManager::ModUpdateState;
			using UpdateStateDef = NetworkManager::ModUpdateStateDef;
			
		private:
			KxVersion m_Version;
			wxDateTime m_UpdateCheckDate;
			UpdateState m_State = UpdateState::Unknown;
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
				return m_State == UpdateState::ModUpdated || m_State == UpdateState::ModFileUpdated;
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
