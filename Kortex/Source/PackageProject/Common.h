#pragma once
#include "stdafx.h"
#include "Utility/EnumClassOperations.h"

namespace Kortex::PackageProject
{
	enum class ContentType
	{
		FileData,
		Images,
		Documents,
	};
	enum class PackageType
	{
		Unknown = -1,
	
		Native,
		Legacy,
		FOModXML,
		FOModCSharp,
		BAIN,
		OMOD,
	};

	enum class ReqState
	{
		Unknown = -1,
		True = 1,
		False = 0
	};
	enum class ObjectFunction
	{
		Invalid = -1,
		None = 0,

		ModActive,
		ModInactive,
		FileExist,
		FileNotExist,
		PluginActive,
		PluginInactive,
	};
	enum class ReqType
	{
		User = 0,
		System = 1,
		Auto = 2,
	};

	enum class Operator
	{
		Invalid = -1,
		None = 0,
	
		Equal,
		NotEqual,
		GreaterThan,
		GreaterThanOrEqual,
		LessThan,
		LessThanOrEqual,
	
		And,
		Or,
	
		MAX,
		MAX_COMPARISON = And,
		MIN = Equal,
	};
	enum class SelectionMode
	{
		Any,
		ExactlyOne, // Only one
		AtLeastOne, // One or more
		AtMostOne, // One or nothing
		All, // All entries must be selected
	};
	enum class TypeDescriptor
	{
		Invalid = -1,

		Optional,
		Required,
		Recommended,
		CouldBeUsable,
		NotUsable,
	};
}

namespace KxEnumClassOperations
{
	KxAllowEnumCastOp(Kortex::PackageProject::ContentType);
	KxAllowEnumCastOp(Kortex::PackageProject::PackageType);

	KxAllowEnumCastOp(Kortex::PackageProject::ReqState);
	KxAllowEnumCastOp(Kortex::PackageProject::ObjectFunction);
	KxAllowEnumCastOp(Kortex::PackageProject::ReqType);

	KxAllowEnumCastOp(Kortex::PackageProject::Operator);
	KxAllowEnumCastOp(Kortex::PackageProject::SelectionMode);
	KxAllowEnumCastOp(Kortex::PackageProject::TypeDescriptor);
}
