#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/Item.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	class SimpleItem: public RTTI::IExtendInterface<SimpleItem, Item>
	{
		private:
			ItemValue m_Value;
			HashStore m_HashStore;
			const bool m_IsUnknown = false;

			mutable std::unique_ptr<KxDataView2::Editor> m_Editor;
			mutable std::optional<wxString> m_CachedViewData;

		protected:
			bool Create(const KxXMLNode& itemNode) override;

			void Clear() override;
			void Read(const ISource& source) override;
			void Write(ISource& source) const override;

		private:
			std::unique_ptr<wxValidator> CreateValidator() const;
			std::unique_ptr<KxDataView2::Editor> CreateEditor() const;

			bool IsReadOnlyComboBox() const
			{
				return HasSamples() && !IsEditable();
			}

		public:
			SimpleItem(ItemGroup& group, const KxXMLNode& itemNode = {});
			SimpleItem(ItemGroup& group, bool isUnknown);

		public:
			size_t GetHash() const override
			{
				return m_HashStore.Get(*this);
			}
			bool IsUnknown() const override
			{
				return m_IsUnknown;
			}
			wxString GetStringRepresentation(ColumnID id) const override;

			const ItemValue& GetValue() const
			{
				return m_Value;
			}
			ItemValue& GetValue()
			{
				return m_Value;
			}
			
		public:
			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;
			bool SetValue(const wxAny& value, KxDataView2::Column& column) override;
			
			KxDataView2::Renderer& GetRenderer(const KxDataView2::Column& column) const override;
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			bool GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const override;
	};
}
