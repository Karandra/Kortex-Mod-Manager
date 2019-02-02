#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ISource.h"
#include "Item.h"
#include "ItemOptions.h"
#include <KxFramework/DataView2/Node.h>
#include <KxFramework/DataView2/TypeAliases.h>
class KxXMLNode;

namespace Kortex
{
	class IConfigManager;
}

namespace Kortex::GameConfig
{
	class Definition;

	class ItemGroup: public RTTI::IExtendInterface<ItemGroup, KxDataView2::Node>, public KxDataView2::TypeAliases
	{
		private:
			Definition& m_Definition;

			wxString m_ID;
			ItemOptions m_Options;
			SourceTypeValue m_SourceType = SourceType::None;
			std::unique_ptr<ISource> m_Source;
			std::unordered_map<size_t, std::unique_ptr<Item>> m_Items;
			bool m_UnknownLoaded = false;

		private:
			void LoadItems(const KxXMLNode& groupNode);
			template<class TItems, class TFunctor> static void CallForEachItem(TItems&& items, TFunctor&& func)
			{
				for (auto& [hash, item]: items)
				{
					func(*item);
				}
			}
			
		public:
			ItemGroup(Definition& definition, const wxString& id, const KxXMLNode& groupNode, const ItemOptions& parentOptions);

		public:
			bool IsOK() const
			{
				return !m_ID.IsEmpty() && m_Source != nullptr;
			}
			wxString GetID() const
			{
				return m_ID;
			}
			const ItemOptions& GetOptions() const
			{
				return m_Options;
			}
			Definition& GetDefinition() const
			{
				return m_Definition;
			}
			IConfigManager& GetManager() const;

			const ISource& GetSource() const
			{
				return *m_Source;
			}
			ISource& GetSource()
			{
				return *m_Source;
			}
			void ReadItems();

			template<class TFunctor> void ForEachItem(TFunctor&& func) const
			{
				CallForEachItem(m_Items, func);
			}
			template<class TFunctor> void ForEachItem(TFunctor&& func)
			{
				CallForEachItem(m_Items, func);
			}

			bool OnLoadInstance(const KxXMLNode& groupNode);
			
			bool HasItem(const Item& item) const
			{
				return m_Items.find(item.GetHash()) != m_Items.end();
			}
			template<class T, class... Args> auto NewItem(Args&&... arg)
			{
				return std::make_unique<T>(std::forward<Args>(arg)...);
			}
			template<class T> T& AddItem(std::unique_ptr<T> item)
			{
				auto[it, inserted] = m_Items.insert_or_assign(item->GetHash(), std::move(item));
				return *static_cast<T*>(it->second.get());
			}
			template<class T, class... Args> T& EmplaceItem(Args&&... arg)
			{
				return AddItem(NewItem<T>(std::forward<Args>(arg)...));
			}
	
		public:
			wxAny GetValue(const Column& column) const override;
	};
}
