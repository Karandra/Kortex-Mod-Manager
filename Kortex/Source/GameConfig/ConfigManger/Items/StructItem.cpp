#include "stdafx.h"
#include "StructItem.h"
#include <KxFramework/DataView2/DataView2.h>
#include <KxFramework/KxStringUtility.h>
#include <regex>

namespace Kortex::GameConfig
{
	void StructItem::Clear()
	{
		for (StructSubItem& subItem: m_SubItems)
		{
			subItem.Clear();
		}
	}
	void StructItem::Read(const ISource& source)
	{
		switch (m_SerializationMode.GetValue())
		{
			case StructSerializationModeID::ElementWise:
			{
				for (StructSubItem& subItem: m_SubItems)
				{
					subItem.Read(source);
				}
				break;
			}
			case StructSerializationModeID::AsString:
			{
				StructSubItem item(*this);
				item.SetTypeID(DataTypeID::String);
				item.Read(source);
				
				ParseFromString(item.GetValue().As<wxString>());
				break;
			}
		};
	}
	void StructItem::Write(ISource& source) const
	{
		switch (m_SerializationMode.GetValue())
		{
			case StructSerializationModeID::ElementWise:
			{
				for (const StructSubItem& subItem: m_SubItems)
				{
					subItem.Write(source);
				}
				break;
			}
			case StructSerializationModeID::AsString:
			{
				ItemValue value;
				value.Assign(FormatToOutput(SerializeFor::Storage));
				
				source.WriteValue(*this, value);
				break;
			}
		};
	}
	void StructItem::ChangeNotify()
	{
		for (StructSubItem& subItem: m_SubItems)
		{
			subItem.ResetCache();
		}
		m_CachedViewValue.reset();
		Item::ChangeNotify();
	}

	void StructItem::ParseFromString(const wxString& sourceString)
	{
		wxString format = GetOptions().GetInputFormat();
		format.Replace(wxS("(%)"), wxS("(.+)"));

		wxRegEx regEx(format, wxRE_ADVANCED|wxRE_ICASE);
		if (regEx.Matches(sourceString) && regEx.GetMatchCount() == m_SubItems.size() + 1)
		{
			for (size_t i = 0; i < m_SubItems.size(); i++)
			{
				StructSubItem& subItem = m_SubItems[i];
				subItem.GetValue().Deserialize(regEx.GetMatch(sourceString, i + 1), subItem);
			}
		}
	}
	wxString StructItem::FormatToOutput(SerializeFor mode) const
	{
		KxFormat formatter(GetOptions().GetOutputFormat());
		for (const StructSubItem& subItem: m_SubItems)
		{
			formatter(subItem.GetValue().Serialize(subItem, mode));
		}
		return formatter;
	}
	size_t StructItem::GetMinOfAllSamples() const
	{
		size_t min = std::numeric_limits<size_t>::max();
		for (const StructSubItem& subItem: m_SubItems)
		{
			min = std::min(min, subItem.GetSamples().GetCount());
		}
		
		return  min != std::numeric_limits<size_t>::max() ? min : 0;
	}

	std::unique_ptr<KxDataView2::ComboBoxEditor> StructItem::CreateEditor() const
	{
		if (!m_StructKindValue.IsDefault())
		{
			auto editor = std::make_unique<KxDataView2::ComboBoxEditor>();
			editor->SetMaxVisibleItems(32);

			const bool isEditable = IsEditable();
			editor->SetEditable(isEditable);
			editor->AutoPopup(!isEditable);

			// Add samples
			auto AddSample = [&editor](const SampleValue& sample, const wxString& value)
			{
				if (sample.HasLabel())
				{
					editor->AddItem(KxString::Format(wxS("%1 - %2"), value, sample.GetLabel()));
				}
				else
				{
					editor->AddItem(value);
				}
			};

			editor->ClearItems();
			switch (m_StructKindValue.GetValue())
			{
				case StructKindID::Default:
				{
					GetSamples().ForEachSample([this, &AddSample](const SampleValue& sample)
					{
						const ItemValue& sampleValue = sample.GetValue();
						AddSample(sample, sampleValue.As<wxString>());
					});
					break;
				}
				case StructKindID::SideBySide:
				{
					if (const size_t min = GetMinOfAllSamples(); min != 0)
					{
						for (size_t i = 0; i < min; ++i)
						{
							KxFormat formatter(GetOptions().GetOutputFormat());
							for (const StructSubItem& subItem: m_SubItems)
							{
								formatter(subItem.GetSamples().GetSampleByIndex(i)->GetValue().Serialize(subItem, SerializeFor::Display));
							}
							editor->AddItem(formatter);
						}	
					}
					break;
				}
			};
			return editor;
		}
		return nullptr;
	}

	StructItem::StructItem(ItemGroup& group, const KxXMLNode& itemNode)
		:IExtendInterface(group, itemNode)
	{
	}
	StructItem::StructItem(ItemGroup& group, bool isUnknown)
		:IExtendInterface(group)
	{
	}

	bool StructItem::Create(const KxXMLNode& itemNode)
	{
		if (itemNode.IsOK())
		{
			// Load struct options
			const KxXMLNode structOptions = itemNode.GetFirstChildElement(wxS("Options"));
			m_SerializationMode.FromString(structOptions.GetFirstChildElement(wxS("SerializationMode")).GetAttribute(wxS("Value")));
			m_StructKindValue.FromString(structOptions.GetFirstChildElement(wxS("StructKind")).GetAttribute(wxS("Value")));

			const KxXMLNode subItemsNode = itemNode.GetFirstChildElement(wxS("SubItems"));
			m_SubItems.reserve(subItemsNode.GetChildrenCount());

			// Load sub-items
			for (KxXMLNode node = subItemsNode.GetFirstChildElement(wxS("Item")); node.IsOK(); node = node.GetNextSiblingElement(wxS("Item")))
			{
				TypeID type;
				if (type.FromString(node.GetAttribute(wxS("Type"))) && type.IsScalarType())
				{
					StructSubItem& subItem = m_SubItems.emplace_back(*this, node);
					if (!subItem.Create(node))
					{
						m_SubItems.pop_back();
					}
				}
			}
		}
		return IsOK();
	}
	wxString StructItem::GetViewString(ColumnID id) const
	{
		if (id == ColumnID::Type)
		{
			if (!m_CachedViewType)
			{
				wxString value;
				for (const StructSubItem& subItem: m_SubItems)
				{
					wxString type = subItem.GetTypeID().ToString();
					if (value.IsEmpty())
					{
						value = type;
					}
					else
					{
						value = Kx::Utility::String::ConcatWithSeparator(wxS(", "), value, type);
					}
				}
				m_CachedViewType = KxString::Format(wxS("struct<%1>"), value, 1, 4);
			}
			return *m_CachedViewType;
		}
		else if (id == ColumnID::Value)
		{
			if (!m_CachedViewValue)
			{
				wxString finalValue;
				if (GetOptions().HasOutputFormat())
				{
					finalValue = FormatToOutput(SerializeFor::Display);
				}
				else
				{
					for (const StructSubItem& subItem: m_SubItems)
					{
						wxString value = subItem.GetValue().Serialize(subItem);
						if (finalValue.IsEmpty())
						{
							finalValue = value;
						}
						else
						{
							finalValue = Kx::Utility::String::ConcatWithSeparator(wxS(", "), finalValue, value);
						}
					}
					finalValue = KxString::Format(wxS("{%1}"), finalValue);
				}

				const SampleValue* sample = GetSamples().FindSampleByValue(ItemValue(finalValue));
				if (sample && sample->HasLabel())
				{
					m_CachedViewValue = KxString::Format(wxS("%1 - %2"), finalValue, sample->GetLabel());
				}
				else
				{
					m_CachedViewValue = finalValue;
				}
			}
			return *m_CachedViewValue;
		}
		return Item::GetViewString(id);
	}
	void StructItem::OnActivate(KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			Edit(column);
		}
		Item::OnActivate(column);
	}
	void StructItem::OnAttachToView()
	{
		for (StructSubItem& subItem: m_SubItems)
		{
			AttachChild(&subItem, GetChildrenCount());
		}
	}

	wxAny StructItem::GetValue(const KxDataView2::Column& column) const
	{
		return Item::GetValue(column);
	}
	wxAny StructItem::GetEditorValue(const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			switch (m_StructKindValue.GetValue())
			{
				case StructKindID::Default:
				{
					size_t index = std::numeric_limits<size_t>::max();
					size_t counter = 0;
					const wxString currentValue = FormatToOutput(SerializeFor::Storage);

					GetSamples().ForEachSample([this, &index, &counter, &currentValue](const SampleValue& sample)
					{
						if (sample.GetValue().As<wxString>() == currentValue && index != 0)
						{
							index = counter;
						}
						counter++;
					});
					return index < counter ? index : 0;
				}
				case StructKindID::SideBySide:
				{
					if (const size_t min = GetMinOfAllSamples(); min != 0)
					{
						const wxString currentValue = FormatToOutput(SerializeFor::Storage);
						for (size_t i = 0; i < min; ++i)
						{
							KxFormat formatter(GetOptions().GetOutputFormat());
							for (const StructSubItem& subItem: m_SubItems)
							{
								formatter(subItem.GetSamples().GetSampleByIndex(i)->GetValue().Serialize(subItem, SerializeFor::Storage));
							}
							if (formatter == currentValue)
							{
								return i;
							}
						}
					}
				}
			};
		}
		return {};
	}
	bool StructItem::SetValue(const wxAny& value, KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value && value.CheckType<int>())
		{
			const size_t selectedIndex = value.As<int>();

			switch (m_StructKindValue.GetValue())
			{
				case StructKindID::Default:
				{
					const SampleValue* sampleValue = GetSamples().GetSampleByIndex(selectedIndex);
					if (sampleValue)
					{
						const wxString oldValue = FormatToOutput(SerializeFor::Storage);
						ParseFromString(sampleValue->GetValue().As<wxString>());
						if (oldValue != FormatToOutput(SerializeFor::Storage))
						{
							ChangeNotify();
							return true;
						}
					}
					return false;
				}
				case StructKindID::SideBySide:
				{
					const wxString oldValue = FormatToOutput(SerializeFor::Storage);
					for (StructSubItem& subItem: m_SubItems)
					{
						const SampleValue* sampleValue = subItem.GetSamples().GetSampleByIndex(selectedIndex);
						if (sampleValue)
						{
							subItem.GetValue().Deserialize(sampleValue->GetValue().Serialize(subItem), subItem);
						}
					}

					if (FormatToOutput(SerializeFor::Storage) != oldValue)
					{
						ChangeNotify();
						return true;
					}
					return false;
				}
			};
		}
		return false;
	}

	KxDataView2::Editor* StructItem::GetEditor(const KxDataView2::Column& column) const
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
