#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/Item.h"
#include "StructSubItem.h"

namespace Kortex::GameConfig
{
	class StructItem: public RTTI::IExtendInterface<StructItem, Item>
	{
		private:
			HashStore m_HashStore;
			std::vector<StructSubItem> m_SubItems;
			StructSerializationModeValue m_SerializationMode;

			mutable std::unique_ptr<KxDataView2::Editor> m_Editor;
			mutable std::optional<wxString> m_CachedViewType;
			mutable std::optional<wxString> m_CachedViewValue;

		protected:
			bool Create(const KxXMLNode& itemNode) override;

			void Clear() override;
			void Read(const ISource& source) override;
			void Write(ISource& source) const override;
			void ChangeNotify() override;

		private:
			std::unique_ptr<KxDataView2::Editor> CreateEditor() const;
			bool IsComboBoxEditor() const
			{
				return HasSamples();
			}

			template<class TItems, class TFunctor> static void DoForEachItem(TItems&& items, TFunctor&& func)
			{
				for (auto&& item: items)
				{
					func(item);
				}
			}

		public:
			StructItem(ItemGroup& group, const KxXMLNode& itemNode = {});
			StructItem(ItemGroup& group, bool isUnknown);

		public:
			bool IsOK() const override
			{
				return !m_SubItems.empty() && Item::IsOK();
			}
			size_t GetHash() const override
			{
				return m_HashStore.Get(*this);
			}
			bool IsUnknown() const override
			{
				return false;
			}
			StructSerializationModeValue GetSerializationMode() const
			{
				return m_SerializationMode;
			}

			void OnAttachToView() override;
			wxString GetStringRepresentation(ColumnID id) const override;
			
			template<class TFunctor> void ForEachSubItem(TFunctor&& func) const
			{
				DoForEachItem(m_SubItems, func);
			}
			template<class TFunctor> void ForEachSubItem(TFunctor&& func)
			{
				DoForEachItem(m_SubItems, func);
			}

		public:
			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;
			bool SetValue(const wxAny& value, KxDataView2::Column& column) override;

			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			void OnActivate(KxDataView2::Column& column) override;
	};
}
