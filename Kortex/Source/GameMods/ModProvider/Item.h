#pragma once
#include "stdafx.h"
#include "Network/NetworkConstants.h"
#include "Network/INetworkProvider.h"
#include "GameInstance/GameID.h"

namespace Kortex::ModProvider
{
	class Store;
	class Item
	{
		friend class Store;

		private:
			using TID = std::variant<wxString, INetworkProvider*>;
			using TData = std::variant<wxString, Network::ModID>;

		private:
			TID m_ID = nullptr;
			TData m_Data = Network::InvalidModID;

		public:
			Item() = default;

			Item(const wxString& name, const wxString& url)
				:m_ID(name), m_Data(url)
			{
			}
			Item(const wxString& name, Network::ModID id)
				:m_ID(name), m_Data(id)
			{
			}

			Item(INetworkProvider* provider, const wxString& url)
				:m_ID(provider), m_Data(url)
			{
			}
			Item(INetworkProvider* provider, Network::ModID id)
				:m_ID(provider), m_Data(id)
			{
			}

		public:
			bool IsOK() const
			{
				return (HasProvider() || HasName()) && (HasModID() || HasURL());
			}
			void Load(const KxXMLNode& node);
			void Save(KxXMLNode& node) const;

			INetworkProvider* GetProvider() const;
			void SetProvider(INetworkProvider* provider)
			{
				m_ID = provider;
			}
			template<class T> void SetProvider()
			{
				m_ID = T::GetInstance();
			}
			bool TryGetProvider(INetworkProvider*& provider) const
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

			Network::ModID GetModID() const;
			void SetModID(Network::ModID id)
			{
				m_Data = id;
			}
			bool TryGetModID(Network::ModID& id) const
			{
				id = GetModID();
				return id != Network::InvalidModID;
			}
			bool HasModID() const
			{
				return GetModID() != Network::InvalidModID;
			}
	};
}
