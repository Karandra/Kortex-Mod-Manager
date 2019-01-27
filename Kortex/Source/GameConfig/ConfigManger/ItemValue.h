#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

namespace Kortex::GameConfig
{
	class ItemValue
	{
		private:
			wxAny m_Value;
			wxString m_Label;
			DataType m_Type;

		private:
			void FromString(const wxString& stringValue);
			void AsBool(TypeID inputType, const wxString& stringValue);
			void AsSignedInteger(TypeID inputType, const wxString& stringValue);
			void AsUnsignedInteger(TypeID inputType, const wxString& stringValue);
			void AsFloat(TypeID inputType, const wxString& stringValue);
			void AsString(TypeID inputType, const wxString& stringValue);

		public:
			ItemValue() = default;
			template<class T> ItemValue(const DataType& type, T&& value)
				:m_Type(type), m_Value(value)
			{
			}
			template<> ItemValue(const DataType& type, const wxString& value)
				:m_Type(type)
			{
				FromString(value);
			}

		public:
			bool IsOk() const
			{
				return m_Type.IsOK();
			}
			DataType GetType() const
			{
				return m_Type;
			}
			void SetType(DataType type)
			{
				m_Type = type;
			}

			wxString GetLabel() const
			{
				if (m_Label.IsEmpty())
				{
					wxString label;
					m_Value.GetAs<wxString>(&label);
					return label;
				}
				return m_Label;
			}
			void SetLabel(const wxString& label)
			{
				m_Label = label;
			}
			
			bool HasValue() const
			{
				return !m_Value.IsNull();
			}
			const wxAny& GetValue() const
			{
				return m_Value;
			}
			void SetValue(const wxAny& value)
			{
				m_Value = value;
			}
			void RemoveValue()
			{
				m_Value.MakeNull();
			}
			template<class T> bool SetValue(T&& value)
			{
				m_Value = value;
				return HasValue();
			}
			template<> bool SetValue(const wxString& value)
			{
				FromString(value);
				return HasValue();
			}
	};
}
