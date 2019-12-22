#include "stdafx.h"
#include "SimpleItem.h"
#include "GameConfig/ConfigManger/ItemGroup.h"
#include "Utility/Common.h"
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
	void SimpleItem::Clear()
	{
		ResetCache();
		m_Value.MakeNull();
	}
	void SimpleItem::Read(const ISource& source)
	{
		if (!source.ReadValue(*this, m_Value))
		{
			m_Value.MakeNull();
		}
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
				comboBox->SetMaxVisibleItems(16);
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

	SimpleItem::SimpleItem(ItemGroup& group, const KxXMLNode& itemNode)
		:ExtendInterface(group, itemNode)
	{
	}
	SimpleItem::SimpleItem(ItemGroup& group, bool isUnknown)
		:ExtendInterface(group), m_IsUnknown(isUnknown)
	{
		GetOptions().CopyIfNotSpecified(group.GetOptions(), GetDataType());
	}

	bool SimpleItem::Create(const KxXMLNode& itemNode)
	{
		RegisterAsKnown();
		return IsOK();
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
							return Utility::MakeBracketedLabel(GetManager().GetTranslator().GetString(wxS("ConfigManager.View.EmptyStringValue")));
						}
						else
						{
							return Utility::MakeNoneLabel();
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
			if (HasAction())
			{
				const wxString oldVaue = m_Value.Serialize(*this);

				bool isInvoked = false;
				if (auto action = GetManager().QueryAction(GetActionName()))
				{
					action->Invoke(*this, m_Value);
					isInvoked = true;
				}
				else
				{
					isInvoked = IAction::InvokeIntrinsicAction(GetIntrinsicAction(), *this, m_Value);
				}

				if (isInvoked && oldVaue != m_Value.Serialize(*this))
				{
					ChangeNotify();
					Refresh(column);
				}
			}
			else
			{
				if (GetTypeID().IsBool())
				{
					m_Value.Assign(!m_Value.As<bool>());
					ChangeNotify();
					Refresh(column);
				}
				else
				{
					Edit(column);
				}
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
				if (GetSamples().FindSampleByValue(m_Value, &index))
				{
					return (int)index;
				}
			}
			return m_Value.Serialize(*this);
		}
		return {};
	}
	bool SimpleItem::SetValue(KxDataView2::Column& column, const wxAny& value)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			if (GetTypeID().IsBool())
			{
				m_Value.Assign(!m_Value.As<bool>());
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
		if (column.GetID<ColumnID>() == ColumnID::Value && GetOptions().GetEditableBehavior() != EditableBehaviorID::Inert)
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
