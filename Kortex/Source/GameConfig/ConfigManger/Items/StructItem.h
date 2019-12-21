#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/Item.h"
#include "StructSubItem.h"

namespace KxDataView2
{
	class KX_API ComboBoxEditor;
}

namespace Kortex::GameConfig
{
	enum class StructKindID
	{
		None = 0,
		Default,
		SideBySide,
	};
	struct StructKindDef: public KxIndexedEnum::Definition<StructKindDef, StructKindID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{StructKindID::None, wxS("None")},
			{StructKindID::Default, wxS("Default")},
			{StructKindID::SideBySide, wxS("SideBySide")},
		};
	};
	using StructKindValue = KxIndexedEnum::Value<StructKindDef, StructKindID::None>;
}

namespace Kortex::GameConfig
{
	enum class StructSerializationModeID: uint32_t
	{
		ElementWise = 0,
		AsString
	};
	struct StructSerializationModeDef: public KxIndexedEnum::Definition<StructSerializationModeDef, StructSerializationModeID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{StructSerializationModeID::ElementWise, wxS("ElementWise")},
			{StructSerializationModeID::AsString, wxS("AsString")},
		};
	};
	using StructSerializationModeValue = KxIndexedEnum::Value<StructSerializationModeDef, StructSerializationModeID::ElementWise>;
}

namespace Kortex::GameConfig
{
	class StructItem: public KxRTTI::ExtendInterface<StructItem, Item>
	{
		KxDecalreIID(StructItem, {0x3ca50288, 0xb99d, 0x4eb8, {0xbd, 0x81, 0x79, 0xdf, 0x45, 0x6b, 0xf8, 0x7}});

		friend class StructSubItem;

		private:
			std::vector<StructSubItem> m_SubItems;
			StructSerializationModeValue m_SerializationMode;
			StructKindValue m_StructKindValue;

			mutable std::unique_ptr<KxDataView2::ComboBoxEditor> m_Editor;
			mutable std::optional<wxString> m_CachedViewType;
			mutable std::optional<wxString> m_CachedViewValue;

		protected:
			void Clear() override;
			void Read(const ISource& source) override;
			void Write(ISource& source) const override;
			void ChangeNotify() override;

		private:
			void ParseFromString(const wxString& sourceString);
			wxString FormatToOutput(SerializeFor mode) const;
			size_t GetMinOfAllSamples() const;
		
			std::unique_ptr<KxDataView2::ComboBoxEditor> CreateEditor() const;
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
			bool Create(const KxXMLNode& itemNode = {}) override;
			bool IsOK() const override
			{
				return !m_SubItems.empty() && Item::IsOK();
			}
			bool IsUnknown() const override
			{
				return false;
			}
			
			StructSerializationModeValue GetSerializationMode() const
			{
				return m_SerializationMode;
			}
			StructKindValue GetStructKind() const
			{
				return m_StructKindValue;
			}

			template<class TFunctor> void ForEachSubItem(TFunctor&& func) const
			{
				DoForEachItem(m_SubItems, func);
			}
			template<class TFunctor> void ForEachSubItem(TFunctor&& func)
			{
				DoForEachItem(m_SubItems, func);
			}

		public:
			wxString GetViewString(ColumnID id) const override;
			void OnActivate(KxDataView2::Column& column) override;
			void OnAttachToView() override;

			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Column& column, const wxAny& value) override;

			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
	};
}
