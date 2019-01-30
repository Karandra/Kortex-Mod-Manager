#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

namespace Kortex::GameConfig
{
	class ItemOptions;

	class ItemValue
	{
		private:
			wxAny m_Value;
			DataType m_Type;

		private:
			void FromString(const wxString& stringValue, const ItemOptions& options);
			void AsBool(TypeID inputType, const wxString& stringValue);
			void AsSignedInteger(TypeID inputType, const wxString& stringValue);
			void AsUnsignedInteger(TypeID inputType, const wxString& stringValue);
			void AsFloat(TypeID inputType, const wxString& stringValue);
			void AsString(TypeID inputType, const wxString& stringValue);

			wxString ToString(const ItemOptions& options) const;
			wxString FromBool(TypeID outputType, const ItemOptions& options) const;
			wxString FromSignedInteger(TypeID outputType, const ItemOptions& options) const;
			wxString FromUnsignedInteger(TypeID outputType, const ItemOptions& options) const;
			wxString FromFloat(TypeID outputType, const ItemOptions& options) const;
			wxString FromString(TypeID outputType, const ItemOptions& options) const;

		public:
			ItemValue() = default;
			ItemValue(const DataType& type)
				:m_Type(type)
			{
			}
			template<class T> ItemValue(const DataType& type, const ItemOptions& options, T&& value)
				:m_Type(type), m_Value(value)
			{
			}
			template<> ItemValue(const DataType& type, const ItemOptions& options, const wxString& value)
				:m_Type(type)
			{
				FromString(value, options);
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
			
			bool IsNull() const
			{
				return m_Value.IsNull();
			}
			void MakeNull()
			{
				m_Value.MakeNull();
			}

			template<class T> bool As(T& value) const
			{
				return m_Value.GetAs(&value);
			}
			template<> bool As(wxAny& value) const
			{
				value = m_Value;
				return true;
			}

			template<class T> T As() const
			{
				static_assert(std::is_default_constructible_v<T>);

				T value;
				return m_Value.GetAs(&value);
			}
			template<> wxAny As() const
			{
				return m_Value;
			}

			template<class T> bool Assign(T&& value)
			{
				m_Value = value;
				return !IsNull();
			}
			template<> bool Assign(wxAny&& value)
			{
				m_Value = std::move(value);
				value.MakeNull();
				return !IsNull();
			}
	};
}
