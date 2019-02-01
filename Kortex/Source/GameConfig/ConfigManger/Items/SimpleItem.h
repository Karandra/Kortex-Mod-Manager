#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/Item.h"

namespace Kortex::GameConfig
{
	class SimpleItem: public RTTI::IImplementation<Item>
	{
		private:
			ItemValue m_Value;

		protected:
			bool Create(const KxXMLNode& itemNode) override;

			void Clear() override;
			void Read(const ISource& source) override;
			void Write(ISource& source) const override;

		public:
			SimpleItem(ItemGroup& group, const KxXMLNode& itemNode = {});

		public:
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
