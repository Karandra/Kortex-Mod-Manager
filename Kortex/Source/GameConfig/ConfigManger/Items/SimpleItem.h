#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/Item.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	class SimpleItem: public RTTI::IImplementation<Item>
	{
		private:
			ItemValue m_Value;
			HashStore m_HashStore;
			const bool m_IsUnknown = false;

			mutable KxDataView2::TextRenderer m_Renderer;
			mutable KxDataView2::TextEditor m_Editor;

		protected:
			bool Create(const KxXMLNode& itemNode) override;

			void Clear() override;
			void Read(const ISource& source) override;
			void Write(ISource& source) const override;

		public:
			SimpleItem(ItemGroup& group, const KxXMLNode& itemNode = {});
			SimpleItem(ItemGroup& group, bool isUnknown);

		public:
			size_t GetHash() const override
			{
				return m_HashStore.Get(*this);
			}
			bool IsUnknown() const
			{
				return m_IsUnknown;
			}

			const ItemValue& GetValue() const
			{
				return m_Value;
			}
			ItemValue& GetValue()
			{
				return m_Value;
			}
	
		public:
			wxAny GetValue(const Column& column) const override;
			Renderer& GetRenderer(const Column& column) const override;
			Editor* GetEditor(const Column& column) const override;
			bool GetAttributes(CellAttributes& attributes, const CellState& cellState, const Column& column) const override;
	};
}