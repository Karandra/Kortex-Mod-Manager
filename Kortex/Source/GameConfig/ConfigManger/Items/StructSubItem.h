#pragma once
#include "stdafx.h"
#include "SimpleItem.h"

namespace Kortex::GameConfig
{
	class StructItem;

	class StructSubItem: public RTTI::IExtendInterface<StructSubItem, SimpleItem>
	{
		friend class StructItem;

		private:
			StructItem& m_Struct;

		public:
			StructSubItem(StructItem& structItem, const KxXMLNode& itemNode = {});

		public:
			bool IsOK() const override;
			wxString GetViewString(ColumnID id) const override;
			wxString GetPath() const override;

			const StructItem& GetStruct() const
			{
				return m_Struct;
			}
			StructItem& GetStruct()
			{
				return m_Struct;
			}
	};
}
