#include "stdafx.h"
#include "SimpleItem.h"
#include "Utility/KAux.h"
#include <KxFramework/DataView2/DataView2.h>

namespace
{
	using Kortex::GameConfig::ItemSamples;

	template<class TValidator> void SetValidatorBounds(TValidator& validator, const ItemSamples& samples)
	{
		using T = typename TValidator::ValueType;

		T min = 0;
		T max = 0;
		samples.GetBoundValues(min, max);
		validator.SetMin(min);
		validator.SetMax(max);
	}
}

namespace Kortex::GameConfig
{
	bool SimpleItem::Create(const KxXMLNode& itemNode)
	{
		return IsOK();
	}

	void SimpleItem::Clear()
	{
		m_Value.MakeNull();
	}
	void SimpleItem::Read(const ISource& source)
	{
		source.ReadValue(*this, m_Value);
	}
	void SimpleItem::Write(ISource& source) const
	{
		source.WriteValue(*this, m_Value);
	}
	
	void SimpleItem::ResetCache()
	{
		m_CachedViewData.reset();
	}
	void SimpleItem::ChangeNotify()
	{
		ResetCache();
		Item::ChangeNotify();
	}

	std::unique_ptr<wxValidator> SimpleItem::CreateValidator() const
	{
		const ItemOptions& options = GetOptions();
		const TypeID type = GetTypeID();

		if (type.IsSignedInteger())
		{
			auto validator = std::make_unique<wxIntegerValidator<int64_t>>();
			SetValidatorBounds(*validator, GetSamples());
			return validator;
		}
		else if (type.IsUnsignedInteger())
		{
			auto validator = std::make_unique<wxIntegerValidator<uint64_t>>();
			SetValidatorBounds(*validator, GetSamples());
			return validator;
		}
		else if (type.IsFloat())
		{
			auto validator = std::make_unique<wxFloatingPointValidator<double>>();
			SetValidatorBounds(*validator, GetSamples());

			if (options.HasPrecision())
			{
				validator->SetPrecision(options.GetPrecision());
			}
			return validator;
		}
		return nullptr;
	}
	std::unique_ptr<KxDataView2::Editor> SimpleItem::CreateEditor() const
	{
		const TypeID type = GetTypeID();
		if (type.IsInteger() || type.IsFloat() || type.IsString())
		{
			std::unique_ptr<KxDataView2::Editor> editor;
			const bool isEditable = IsEditable();

			if (HasSamples())
			{
				auto comboBox = std::make_unique<KxDataView2::ComboBoxEditor>();
				comboBox->SetMaxVisibleItems(32);
				comboBox->AutoPopup(!isEditable);

				// Add samples
				comboBox->ClearItems();
				const ItemSamples& samples = GetSamples();
				if (!samples.IsEmpty())
				{
					samples.ForEachSample([this, &comboBox](const SampleValue& sample)
					{
						const ItemValue& value = sample.GetValue();
						if (sample.HasLabel())
						{
							comboBox->AddItem(KxString::Format(wxS("%1 - %2"), value.Serialize(*this), sample.GetLabel()));
						}
						else
						{
							comboBox->AddItem(value.Serialize(*this));
						}
					});
				}
				editor = std::move(comboBox);
			}
			else
			{
				editor = std::make_unique<KxDataView2::TextEditor>();
			}

			editor->SetEditable(isEditable);
			editor->SetValidator(CreateValidator());
			return editor;
		}
		return nullptr;
	}

	SimpleItem::SimpleItem(ItemGroup& group, const KxXMLNode& itemNode, bool allowLoadSamples)
		:IExtendInterface(group, itemNode, allowLoadSamples)
	{
	}
	SimpleItem::SimpleItem(ItemGroup& group, bool isUnknown)
		:IExtendInterface(group), m_IsUnknown(isUnknown)
	{
	}

	wxString SimpleItem::GetViewString(ColumnID id) const
	{
		if (id == ColumnID::Value)
		{
			if (!m_CachedViewData)
			{
				auto FormatValue = [this](const ItemValue& value)
				{
					wxString serializedValue = value.Serialize(*this, SerializeFor::Display);

					if (serializedValue.IsEmpty())
					{
						if (value.GetType().IsString())
						{
							return KAux::MakeBracketedLabel(GetManager().GetTranslator().GetString(wxS("ConfigManager.View.EmptyStringValue")));
						}
						else
						{
							return KAux::MakeNoneLabel();
						}
					}
					return serializedValue;
				};

				const SampleValue* sampleValue = GetSamples().FindSampleByValue(m_Value);
				if (sampleValue && sampleValue->HasLabel())
				{
					m_CachedViewData = KxString::Format(wxS("%1 - %2"), FormatValue(m_Value), sampleValue->GetLabel());
				}
				else
				{
					m_CachedViewData = FormatValue(m_Value);
				}
			}
			return *m_CachedViewData;
		}
		return Item::GetViewString(id);
	}
	void SimpleItem::OnActivate(KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			if (GetTypeID().IsBool())
			{
				m_Value = !m_Value.As<bool>();
				m_CachedViewData.reset();
				Refresh(column);
			}
			else
			{
				Edit(column);
			}
			return;
		}
		Item::OnActivate(column);
	}

	wxAny SimpleItem::GetValue(const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			wxString value = GetViewString(ColumnID::Value);
			if (GetTypeID().IsBool())
			{
				using namespace KxDataView2;
				if (!m_Value.IsNull())
				{
					return BitmapTextToggleValue(m_Value.As<bool>(), value, wxNullBitmap, ToggleType::CheckBox);
				}
				else
				{
					return BitmapTextToggleValue(value, wxNullBitmap, ToggleState::Indeterminate, ToggleType::CheckBox);
				}
			}
			return value;
		}
		return Item::GetValue(column);
	}
	wxAny SimpleItem::GetEditorValue(const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			if (IsComboBoxEditor() && !IsEditable())
			{
				size_t index = 0;
				GetSamples().FindSampleByValue(m_Value, &index);
				return index;
			}
			else
			{
				return m_Value.Serialize(*this);
			}
		}
		return {};
	}
	bool SimpleItem::SetValue(const wxAny& value, KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			if (GetTypeID().IsBool())
			{
				m_Value = !m_Value.As<bool>();
				ChangeNotify();
				return true;
			}
			else if (IsComboBoxEditor() && value.CheckType<int>())
			{
				const SampleValue* sampleValue = GetSamples().GetSampleByIndex(value.As<int>());
				if (sampleValue)
				{
					wxString serialized = sampleValue->GetValue().Serialize(*this);
					if (serialized != m_Value.Serialize(*this))
					{
						m_Value.Deserialize(serialized, *this);
						ChangeNotify();
						return true;
					}
					return false;
				}
			}

			// Any other variant
			const wxString oldData = m_Value.Serialize(*this);

			wxString data;
			value.GetAs(&data);
			m_Value.Deserialize(data, *this);

			if (m_Value.Serialize(*this) != oldData)
			{
				ChangeNotify();
				return true;
			}
		}
		return false;
	}

	KxDataView2::Editor* SimpleItem::GetEditor(const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			if (!m_Editor)
			{
				m_Editor = CreateEditor();
			}
			return m_Editor.get();
		}
		return nullptr;
	}
}
