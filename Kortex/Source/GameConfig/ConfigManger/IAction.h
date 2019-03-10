#pragma once
#include "stdafx.h"
#include "Common.h"
#include "Item.h"
#include "ItemValue.h"
#include "ItemSamples.h"

namespace Kortex::GameConfig
{
	enum class ActionID
	{
		None,

		BrowseFile,
		BrowseFolder,
		PickColor,
	};
	class ActionDef: public KxIndexedEnum::Definition<ActionDef, ActionID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{ActionID::None, wxS("None")},
			{ActionID::BrowseFile, wxS("BrowseFile")},
			{ActionID::BrowseFolder, wxS("BrowseFolder")},
			{ActionID::PickColor, wxS("PickColor")},
		};
	};
	using ActionValue = KxIndexedEnum::Value<ActionDef, ActionID::None>;
}

namespace Kortex::GameConfig
{
	class IAction: public RTTI::IInterface<IAction>
	{
		public:
			static bool InvokeAction(ActionID id, Item& item, ItemValue& value);

		public:
			IAction() = default;
			IAction(IAction&&) = delete;
			IAction(const IAction&) = delete;
			virtual ~IAction() = default;

		public:
			virtual void Invoke(Item& item, ItemValue& value) = 0;
	
		public:
			IAction& operator=(const IAction&) = delete;
			IAction& operator=(IAction&&) = delete;
	};
}
