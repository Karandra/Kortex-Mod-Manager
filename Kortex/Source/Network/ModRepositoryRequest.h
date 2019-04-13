#pragma once
#include "Common.h"
#include "GameInstance/GameID.h"
#include "NetworkModInfo.h"

namespace Kortex
{
	class ModRepositoryRequest
	{
		private:
			NetworkModInfo m_ModInfo;
			GameID m_GameID;
			wxAny m_ExtraInfo;

		public:
			ModRepositoryRequest() = default;
			ModRepositoryRequest(ModID id)
				:m_ModInfo(id)
			{
			}
			ModRepositoryRequest(ModID modID, ModFileID fileID)
				:m_ModInfo(modID, fileID)
			{
			}
			ModRepositoryRequest(ModID modID, ModFileID fileID, const GameID& gameID)
				:m_ModInfo(modID, fileID), m_GameID(gameID)
			{
			}
			ModRepositoryRequest(NetworkModInfo modInfo, const GameID& gameID = {})
				:m_ModInfo(modInfo), m_GameID(gameID)
			{
			}

		public:
			const NetworkModInfo& GetModInfo() const
			{
				return m_ModInfo;
			}

			bool HasModID() const
			{
				return m_ModInfo.HasModID();
			}
			ModID GetModID() const
			{
				return m_ModInfo.GetModID();
			}

			bool HasFileID() const
			{
				return m_ModInfo.HasFileID();
			}
			ModFileID GetFileID() const
			{
				return m_ModInfo.GetFileID();
			}

			bool HasGameID() const
			{
				return m_GameID.IsOK();
			}
			GameID GetGameID() const
			{
				return m_GameID;
			}

			bool HasExtraInfo() const
			{
				return !m_ExtraInfo.IsNull();
			}
			template<class T> T GetExtraInfo() const
			{
				if (HasExtraInfo())
				{
					T extraInfo;
					if (GetExtraInfo(extraInfo))
					{
						return extraInfo;
					}
				}
				return {};
			}
			template<class T> bool GetExtraInfo(T&& extraInfo) const
			{
				return HasExtraInfo() && m_ExtraInfo.GetAs(&extraInfo);
			}
			template<class T> void SetExtraInfo(T&& extraInfo)
			{
				m_ExtraInfo = extraInfo;
			}
	};
}
