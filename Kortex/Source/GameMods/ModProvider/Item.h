#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/INetworkModSource.h"
#include "Network/NetworkModInfo.h"
#include "GameInstance/GameID.h"

namespace Kortex
{
	class ModSourceStore;
	class ModSourceItem
	{
		friend class ModSourceStore;

		private:
			using TID = std::variant<wxString, INetworkModSource*>;
			using TData = std::variant<wxString, NetworkModInfo>;

		private:
			TID m_ID = nullptr;
			TData m_Data = NetworkModInfo();

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

			ModSourceItem(INetworkModSource* provider, const wxString& url)
				:m_ID(provider), m_Data(url)
			{
			}
			ModSourceItem(INetworkModSource* provider, NetworkModInfo id)
				:m_ID(provider), m_Data(id)
			{
			}

		public:
			bool IsOK() const;
			bool IsEmptyValue() const;
			
			void Load(const KxXMLNode& node);
			void Save(KxXMLNode& node) const;

			INetworkModSource* GetProvider() const;
			void SetProvider(INetworkModSource& provider)
			{
				m_ID = &provider;
			}
			template<class T> void SetProvider()
			{
				m_ID = T::GetInstance();
			}
			bool TryGetProvider(INetworkModSource*& provider) const
			{
				provider = GetProvider();
				return provider != nullptr;
			}
			bool HasProvider() const
			{
				return GetProvider() != nullptr;
			}

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
	};
}
