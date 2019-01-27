#pragma once
#include "stdafx.h"
#include <KxFramework/KxIndexedEnum.h>

namespace Kortex::GameConfig
{
	enum class DataTypeID: uint32_t
	{
		None = 0,
		Any = std::numeric_limits<uint32_t>::max(),

		Int8 = 1 << 0,
		Int16 = 1 << 1,
		Int32 = 1 << 2,
		Int64 = 1 << 3,
		UInt8 = 1 << 4,
		UInt16 = 1 << 5,
		UInt32 = 1 << 6,
		UInt64 = 1 << 7,

		Float32 = 1 << 8,
		Float64 = 1 << 9,

		Bool = 1 << 10,
		String = 1 << 11,

		// Aliases
		Int = Int32,
		UInt = UInt32,
		Float = Float32,
	};
	class DataTypeIDDef: public KxIndexedEnum::Definition<DataTypeIDDef, DataTypeID, wxString, true>
	{
		// Serialization is done with lower-cased versions of these identifiers
		inline static const TItem ms_Index[] =
		{
			{DataTypeID::None, wxS("none")},
			{DataTypeID::Any, wxS("any")},

			{DataTypeID::Int, wxS("int")},
			{DataTypeID::Int8, wxS("int8")},
			{DataTypeID::Int16, wxS("int16")},
			{DataTypeID::Int32, wxS("int32")},
			{DataTypeID::Int64, wxS("int64")},
			{DataTypeID::UInt, wxS("uint")},
			{DataTypeID::UInt8, wxS("uint8")},
			{DataTypeID::UInt16, wxS("uint16")},
			{DataTypeID::UInt32, wxS("uint32")},
			{DataTypeID::UInt64, wxS("uint64")},

			{DataTypeID::Float, wxS("float")},
			{DataTypeID::Float32, wxS("float32")},
			{DataTypeID::Float64, wxS("float64")},

			{DataTypeID::Bool, wxS("bool")},
			{DataTypeID::String, wxS("string")},
		};
	};
	using DataTypeIDValue = KxIndexedEnum::Value<DataTypeIDDef, DataTypeID::None>;
}

namespace Kortex::GameConfig
{
	enum class TypeDetectorID
	{
		None,

		HungarianNotation,
		DataAnalysis
	};
	class TypeDetectorIDDef: public KxIndexedEnum::Definition<TypeDetectorIDDef, TypeDetectorID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{TypeDetectorID::HungarianNotation, wxS("HungarianNotation")},
			{TypeDetectorID::DataAnalysis, wxS("DataAnalysis")},
		};
	};
	using TypeDetectorIDValue = KxIndexedEnum::Value<TypeDetectorIDDef, TypeDetectorID::None>;
}

namespace Kortex::GameConfig
{
	enum class SourceFormat
	{
		None,

		INI,
		XML,
		Registry,
	};
	class SourceFormatDef: public KxIndexedEnum::Definition<SourceFormatDef, SourceFormat, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{SourceFormat::None, wxS("None")},

			{SourceFormat::INI, wxS("INI")},
			//{SourceFormat::XML, wxS("XML")},
			//{SourceFormat::Registry, wxS("Registry")},
		};
	};
	using SourceFormatValue = KxIndexedEnum::Value<SourceFormatDef, SourceFormat::None>;
}

namespace Kortex::GameConfig
{
	enum class SourceType
	{
		None,
		FSPath,
	};
	class SourceTypeDef: public KxIndexedEnum::Definition<SourceTypeDef, SourceType, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{SourceType::None, wxS("None")},
			{SourceType::FSPath, wxS("FSPath")},
		};
	};
	using SourceTypeValue = KxIndexedEnum::Value<SourceTypeDef, SourceType::None>;
}
