#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/INetworkProvider.h"
#include "Network/NetworkModInfo.h"
#include "GameInstance/GameID.h"

namespace Kortex
{
	class ModProviderStore;
	class ModProviderItem
	{
		friend class ModProviderStore;

		private:
			using TID = std::variant<wxString, INetworkProvider*>;
			using TData = std::variant<wxString, NetworkModInfo>;

		private:
			TID m_ID = nullptr;
			TData m_Data = NetworkModInfo();

		public:
			ModProviderItem() = default;

			ModProviderItem(const wxString& name, const wxString& url)
				:m_ID(name), m_Data(url)
			{
			}
			ModProviderItem(const wxString& name, NetworkModInfo id)
				:m_ID(name), m_Data(id)
			{
			}

			ModProviderItem(INetworkProvider* provider, const wxString& url)
				:m_ID(provider), m_Data(url)
			{
			}
			ModProviderItem(INetworkProvider* provider, NetworkModInfo id)
				:m_ID(provider), m_Data(id)
			{
			}

		public:
			bool IsOK() const;
			bool IsEmptyValue() const;
			
			void Load(const KxXMLNode& node);
			void Save(KxXMLNode& node) const;

			INetworkProvider* GetProvider() const;
			void SetProvider(INetworkProvider& provider)
			{
				m_ID = &provider;
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
