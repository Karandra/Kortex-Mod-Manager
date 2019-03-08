#pragma once
#include "Common.h"
#include "GameInstance/GameID.h"

namespace Kortex
{
	class ProviderRequest
	{
		private:
			GameID m_GameID;
			ModID m_ModID;
			ModFileID m_FileID;
			wxAny m_ExtraInfo;

		public:
			ProviderRequest() = default;
			ProviderRequest(ModID id)
				:m_ModID(id)
			{
			}
			ProviderRequest(ModID modID, ModFileID fileID)
				:m_ModID(modID), m_FileID(fileID)
			{
			}
			ProviderRequest(ModID modID, ModFileID fileID, const GameID& gameID)
				:m_ModID(modID), m_FileID(fileID), m_GameID(gameID)
			{
			}
			
		public:
			bool HasGameID() const
			{
				return m_GameID.IsOK();
			}
			GameID GetGameID() const
			{
				return m_GameID;
			}

			bool HasModID() const
			{
				return m_ModID.HasValue();
			}
			ModID GetModID() const
			{
				return m_ModID;
			}

			bool HasFileID() const
			{
				return m_FileID.HasValue();
			}
			ModFileID GetFileID() const
			{
				return m_FileID;
			}

			bool HasExtraInfo() const
			{
				return !m_ExtraInfo.IsNull();
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
