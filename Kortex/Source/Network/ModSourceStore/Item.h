#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "Network/NetworkModInfo.h"
#include "GameInstance/GameID.h"

namespace Kortex
{
	class ModSourceStore;
	class ModSourceItem
	{
		friend class ModSourceStore;

		private:
			using TID = std::variant<wxString, IModNetwork*>;
			using TData = std::variant<wxString, NetworkModInfo>;

		private:
			TID m_ID = nullptr;
			TData m_Data = NetworkModInfo();
			wxDateTime m_LastUpdateCheck;

		public:
			ModSourceItem() = default;

			ModSourceItem(const wxString& name, const wxString& url)
				:m_ID(name), m_Data(url)
			{
			}
			ModSourceItem(const wxString& name, NetworkModInfo id)
				:m_ID(name), m_Data(id)
			{
			}

			ModSourceItem(IModNetwork* modNetwork, const wxString& url)
				:m_ID(modNetwork), m_Data(url)
			{
			}
			ModSourceItem(IModNetwork* modNetwork, NetworkModInfo id)
				:m_ID(modNetwork), m_Data(id)
			{
			}

		public:
			bool IsOK() const;
			bool IsEmptyValue() const;
			
			// Serialization
			void Load(const KxXMLNode& node);
			void Save(KxXMLNode& node) const;

			// Mod network instance
			IModNetwork* GetModNetwork() const;
			void SetModSource(IModNetwork& modNetwork)
			{
				m_ID = &modNetwork;
			}
			template<class T> void SetModSource()
			{
				m_ID = T::GetInstance();
			}
			bool TryGetModNetwork(IModNetwork*& modNetwork) const
			{
				modNetwork = GetModNetwork();
				return modNetwork != nullptr;
			}
			bool HasModNetwork() const
			{
				return GetModNetwork() != nullptr;
			}

			// Name
			wxString GetName() const;
			void SetName(const wxString& name);
			bool TryGetName(wxString& name) const
			{
				name = GetName();
				return !name.IsEmpty();
			}
			bool HasName() const
			{
				return !GetName().IsEmpty();
			}

			// URL
			wxString GetURL(const GameID& gameID = GameIDs::NullGameID) const;
			void SetURL(const wxString& url, const GameID& gameID = GameIDs::NullGameID)
			{
				m_Data = url;
			}
			bool TryGetURL(wxString& url, const GameID& gameID = GameIDs::NullGameID) const
			{
				url = GetURL(gameID);
				return !url.IsEmpty();
			}
			bool HasURL(const GameID& gameID = GameIDs::NullGameID) const
			{
				return !GetURL(gameID).IsEmpty();
			}

			// Network mod info
			NetworkModInfo GetModInfo() const;
			void SetModInfo(NetworkModInfo modInfo)
			{
				m_Data = modInfo;
			}
			bool TryGetModInfo(NetworkModInfo& modInfo) const
			{
				modInfo = GetModInfo();
				return !modInfo.IsEmpty();
			}
			bool HasModInfo() const
			{
				return !GetModInfo().IsEmpty();
			}
	
			// Last update check
			wxDateTime GetLastUpdateCheck() const
			{
				return m_LastUpdateCheck;
			}
			void SetLastUpdateCheck(const wxDateTime& date)
			{
				m_LastUpdateCheck = date;
			}
			bool TryGetLastUpdateCheck(wxDateTime& date) const
			{
				date = GetLastUpdateCheck();
				date.IsValid();
			}
			bool HasLastUpdateCheck() const
			{
				return m_LastUpdateCheck.IsValid();
			}
			bool IsLastUpdateCheckOlderThan(const wxTimeSpan& span) const;
	};
}
