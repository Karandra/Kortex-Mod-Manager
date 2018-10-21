#pragma once
#include "stdafx.h"

namespace KIVariableValueNS
{
	class Override
	{
		public:
			enum _Enum: int
			{
				DoNotChange = -1,

				False = 0,
				True = 1,
			};

		private:
			_Enum m_Value = _Enum::DoNotChange;

		public:
			Override()
				:m_Value(_Enum::DoNotChange)
			{
			}
			Override(_Enum value)
				:m_Value(value)
			{
			}
			Override(bool value)
				:m_Value(value ? _Enum::True : _Enum::False)
			{
			}

		public:
			operator _Enum() const
			{
				return m_Value;
			}
	};

	class Type
	{
		public:
			enum _Enum: int
			{
				DoNotChange = -1,

				None = 0,
				FSPath = 1,
			};

		private:
			_Enum m_Value = _Enum::DoNotChange;

		public:
			Type()
				:m_Value(_Enum::DoNotChange)
			{
			}
			Type(_Enum value)
				:m_Value(value)
			{
			}

		public:
			operator _Enum() const
		{
			return m_Value;
		}
	};
}
