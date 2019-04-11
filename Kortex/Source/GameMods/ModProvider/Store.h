#pragma once
#include "stdafx.h"
#include "Item.h"
#include "Network/Common.h"
#include "Network/NetworkModInfo.h"
#include "Network/INetworkModSource.h"
#include "GameInstance/GameID.h"
#include "Utility/Collection.h"
#include "Utility/KLabeledValue.h"

namespace Kortex
{
	class ModSourceStore
	{
		private:
			template<class TContainer, class TValue> static auto FindItem(TContainer&& items, TValue&& value)
			{
				using DValue = std::remove_const_t<std::decay_t<TValue>>;

				return std::find_if(items.begin(), items.end(), [&value](auto&& item)
				{
					if constexpr(std::is_same_v<DValue, wxString>)
					{
						return item.GetName() == value;
					}
					else if constexpr(std::is_same_v<DValue, INetworkModSource*> || std::is_same_v<DValue, const INetworkModSource*>)
					{
						return item.GetProvider() == value;
					}
					else if constexpr(std::is_same_v<DValue, ModSourceItem::TID>)
					{
						return item.m_ID == value;
					}
					else
					{
						static_assert(false, "invalid object type");
					}
				});
			}
			template<class TContainer, class TValue> static auto FindItemPtr(TContainer&& items, TValue&& value)
			{
				auto it = FindItem(items, value);
				return it != items.end() ? &*it : nullptr;
			}

			enum class LoadMode
			{
				TryAdd,
				Assign
			};
			template<LoadMode t_LoadMode> void LoadHelper(ModSourceStore& store, const KxXMLNode& arrayNode)
			{
				for (KxXMLNode node = arrayNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
				{
					ModSourceItem item;
					item.Load(node);
					if (item.IsOK())
					{
						if constexpr(t_LoadMode == LoadMode::TryAdd)
						{
							store.TryAddItem(std::move(item));
						}
						else if constexpr(t_LoadMode == LoadMode::Assign)
						{
							store.AssignItem(std::move(item));
						}
						else
						{
							static_assert(false, "invalid load mode");
						}
					}
				}
			}

			template<class T> void DoTryAddItem(T&& item)
			{
				if (!FindItemPtr(m_Items, item.m_ID))
				{
					m_Items.emplace_back(item);
				}
			}
			template<class T> void DoAssignItem(T&& item)
			{
				if (ModSourceItem* ptr = FindItemPtr(m_Items, item.m_ID))
				{
					*ptr = item;
				}
				else
				{
					m_Items.emplace_back(item);
				}
			}

		private:
			std::vector<ModSourceItem> m_Items;

		public:
			// Retrieve item
			const ModSourceItem* GetItem(const wxString& name) const;
			ModSourceItem* GetItem(const wxString& name);
			
			const ModSourceItem* GetItem(const INetworkModSource& provider) const;
			ModSourceItem* GetItem(const INetworkModSource& provider);

			template<class T> const ModSourceItem* GetItem() const
			{
				return GetItem(*T::GetInstance());
			}
			template<class T> ModSourceItem* GetItem()
			{
				return GetItem(*T::GetInstance());
			}

			bool HasItem(const wxString& name) const
			{
				return GetItem(name) != nullptr;
			}
			bool HasItem(const INetworkModSource& provider) const
			{
				return GetItem(provider) != nullptr;
			}
			template<class T> bool HasItem()
			{
				return HasItem(*T::GetInstance());
			}
			
			// Adds item if there are none with this name
			void TryAddItem(const ModSourceItem& item)
			{
				DoTryAddItem(item);
			}
			void TryAddItem(ModSourceItem&& item)
			{
				DoTryAddItem(std::move(item));
			}

			ModSourceItem* TryAddWith(const wxString& name, const wxString& url)
			{
				if (!HasItem(name))
				{
					return &m_Items.emplace_back(name, url);
				}
				return nullptr;
			}
			ModSourceItem* TryAddWith(const wxString& name, NetworkModInfo id)
			{
				if (!HasItem(name))
				{
					return &m_Items.emplace_back(name, id);
				}
				return nullptr;
			}
			
			ModSourceItem* TryAddWith(INetworkModSource& provider, const wxString& url)
			{
				if (!HasItem(provider))
				{
					return &m_Items.emplace_back(&provider, url);
				}
				return nullptr;
			}
			ModSourceItem* TryAddWith(INetworkModSource& provider, NetworkModInfo id)
			{
				if (!HasItem(provider))
				{
					return &m_Items.emplace_back(&provider, id);
				}
				return nullptr;
			}

			template<class T> ModSourceItem* TryAddWith(const wxString& url)
			{
				return TryAddWith(*T::GetInstance(), url);
			}
			template<class T> ModSourceItem* TryAddWith(NetworkModInfo id)
			{
				return TryAddWith(*T::GetInstance(), id);
			}
			
			// Assigns new value to existing item or creates new item
			void AssignItem(const ModSourceItem& item)
			{
				DoAssignItem(item);
			}
			void AssignItem(ModSourceItem&& item)
			{
				DoAssignItem(std::move(item));
			}

			ModSourceItem& AssignWith(const wxString& name, const wxString& url);
			ModSourceItem& AssignWith(const wxString& name, NetworkModInfo modInfo);
			
			ModSourceItem& AssignWith(INetworkModSource& provider, const wxString& url);
			ModSourceItem& AssignWith(INetworkModSource& provider, NetworkModInfo modInfo);

			template<class T> ModSourceItem& AssignWith(const wxString& url)
			{
				return AssignWith(*T::GetInstance(), url);
			}
			template<class T> ModSourceItem& AssignWith(NetworkModInfo modInfo)
			{
				return AssignWith(*T::GetInstance(), modInfo);
			}

			bool RemoveItem(const wxString& name);
			bool RemoveItem(const INetworkModSource& provider);

			void Clear()
			{
				m_Items.clear();
			}
			size_t GetSize() const
			{
				return m_Items.size();
			}
			size_t IsEmpty() const
			{
				return m_Items.empty();
			}
			
			template<class TFunctor> void Visit(TFunctor&& func) const
			{
				Utility::Collection::Enumerate(m_Items, func);
			}
			template<class TFunctor> void Visit(TFunctor&& func)
			{
				Utility::Collection::Enumerate(m_Items, func);
			}
			template<class TFunctor> void RemoveIf(TFunctor&& func)
			{
				if (!m_Items.empty())
				{
					for (size_t i = m_Items.size() - 1; i != 0; i--)
					{
						if (func(m_Items[i]))
						{
							m_Items.erase(m_Items.begin() + i);
						}
					}
				}
			}

			wxString GetModURL(const wxString& name, const GameID& gameID = GameIDs::NullGameID) const;
			wxString GetModURL(const INetworkModSource& provider, const GameID& gameID = GameIDs::NullGameID) const;
			template<class T> wxString GetModURL(const GameID& gameID = GameIDs::NullGameID) const
			{
				return GetModURL(*T::GetInstance(), gameID);
			}
			
			KxStringVector GetModURLs(const GameID& gameID = GameIDs::NullGameID) const;
			KLabeledValue::Vector GetModNamedURLs(const GameID& gameID = GameIDs::NullGameID) const;

			void LoadTryAdd(const KxXMLNode& arrayNode);
			void LoadAssign(const KxXMLNode& arrayNode);
			void Save(KxXMLNode& arrayNode) const;
	};
}
