#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "Network/NetworkModInfo.h"
#include "GameInstance/GameID.h"
#include <KxFramework/KxURI.h>

namespace Kortex
{
	class ModSourceStore;
	class ModSourceItem
	{
		friend class ModSourceStore;

		private:
			using TID = std::variant<wxString, IModNetwork*>;
			using TData = std::variant<KxURI, NetworkModInfo>;

		private:
			TID m_ID = nullptr;
			TData m_Data = NetworkModInfo();

		public:
			ModSourceItem() = default;

			ModSourceItem(const wxString& name, const KxURI& uri)
				:m_ID(name), m_Data(uri)
			{
			}
			ModSourceItem(const wxString& name, NetworkModInfo id)
				:m_ID(name), m_Data(id)
			{
			}

			ModSourceItem(IModNetwork* modNetwork, const KxURI& uri)
				:m_ID(modNetwork), m_Data(uri)
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

			// URI
			KxURI GetURI(const GameID& gameID = GameIDs::NullGameID) const;
			void SetURI(const KxURI& uri, const GameID& gameID = GameIDs::NullGameID)
			{
				m_Data = uri;
			}
			bool TryGetURI(KxURI& uri, const GameID& gameID = GameIDs::NullGameID) const
			{
				uri = GetURI(gameID);
				return uri.IsOk();
			}
			bool HasURI(const GameID& gameID = GameIDs::NullGameID) const
			{
				return GetURI(gameID).IsOk();
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
	};
}
