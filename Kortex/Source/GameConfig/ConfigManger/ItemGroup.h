#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ISource.h"
#include "Item.h"
#include "ItemOptions.h"
class KxXMLNode;

namespace Kortex
{
	class IConfigManager;
}

namespace Kortex::GameConfig
{
	class Definition;

	class ItemGroup
	{
		private:
			Definition& m_Definition;

			wxString m_ID;
			ItemOptions m_Options;
			SourceTypeValue m_SourceType = SourceType::None;
			std::unique_ptr<ISource> m_Source;
			std::vector<std::unique_ptr<Item>> m_Items;

		private:
			void LoadItems(const KxXMLNode& groupNode);
			template<class TItems, class TFunctor> static void CallForEachItem(TItems&& items, TFunctor&& func)
			{
				for (auto& item: items)
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

			template<class TFunctor> void ForEachItem(TFunctor&& func) const
			{
				CallForEachItem(m_Items, func);
			}
			template<class TFunctor> void ForEachItem(TFunctor&& func)
			{
				CallForEachItem(m_Items, func);
			}

			bool OnLoadInstance(const KxXMLNode& groupNode);
	};
}
