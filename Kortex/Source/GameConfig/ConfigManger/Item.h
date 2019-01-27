#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"
#include "ItemValue.h"

namespace Kortex
{
	class IConfigManager;
}

namespace Kortex::GameConfig
{
	class ItemGroup;
	class Definition;

	class Item
	{
		private:
			ItemGroup& m_Group;
			wxString m_Category;
			wxString m_Path;
			wxString m_Name;
			ItemValue m_Value;

		public:
			Item(ItemGroup& group, const KxXMLNode& itemNode);
			virtual ~Item() = default;

		public:
			virtual bool IsOK() const;

			IConfigManager& GetManager() const;
			Definition& GetDefinition() const;

			ItemGroup& GetGroup() const
			{
				return m_Group;
			}
			wxString GetCategory() const
			{
				return m_Category;
			}
			wxString GetPath() const
			{
				return m_Path;
			}
			wxString GetName() const
			{
				return m_Name;
			}
			
			DataType GetDataType() const
			{
				return m_Value.GetType();
			}
			const ItemValue& GetValue() const
			{
				return m_Value;
			}
			ItemValue& GetValue()
			{
				return m_Value;
			}
	};
}
