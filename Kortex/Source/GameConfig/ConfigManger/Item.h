#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

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
			TypeID m_Type;

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
			TypeID GetType() const
			{
				return m_Type;
			}
	};
}
