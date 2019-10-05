#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/Item.h"

namespace Kortex::GameConfig
{
	class SimpleItem: public KxRTTI::ExtendInterface<SimpleItem, Item>
	{
		private:
			ItemValue m_Value;
			HashStore m_HashStore;
			const bool m_IsUnknown = false;

			mutable std::unique_ptr<KxDataView2::Editor> m_Editor;
			mutable std::optional<wxString> m_CachedViewData;

		protected:

			void Clear() override;
			void Read(const ISource& source) override;
			void Write(ISource& source) const override;

			void ResetCache();
			void ChangeNotify() override;

		private:
			std::unique_ptr<wxValidator> CreateValidator() const;
			std::unique_ptr<KxDataView2::Editor> CreateEditor() const;
			bool IsComboBoxEditor() const
			{
				return HasSamples();
			}

		public:
			SimpleItem(ItemGroup& group, const KxXMLNode& itemNode = {});
			SimpleItem(ItemGroup& group, bool isUnknown);

		public:
			bool Create(const KxXMLNode& itemNode = {}) override;
			bool IsUnknown() const override
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
			wxString GetViewString(ColumnID id) const override;
			void OnActivate(KxDataView2::Column& column) override;

			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Column& column, const wxAny& value) override;

			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
	};
}
