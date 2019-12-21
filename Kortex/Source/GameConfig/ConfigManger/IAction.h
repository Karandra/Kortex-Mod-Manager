#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ItemValue.h"
#include "ItemSamples.h"

namespace Kortex::GameConfig
{
	enum class IntrinsicActionID
	{
		None,

		BrowseFile,
		BrowseFolder,
		PickColor,
	};
	struct IntrinsicActionDef: public KxIndexedEnum::Definition<IntrinsicActionDef, IntrinsicActionID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{IntrinsicActionID::None, wxS("None")},
			{IntrinsicActionID::BrowseFile, wxS("Intrinsic/BrowseFile")},
			{IntrinsicActionID::BrowseFolder, wxS("Intrinsic/BrowseFolder")},
			{IntrinsicActionID::PickColor, wxS("Intrinsic/PickColor")},
		};
	};
	using IntrinsicActionValue = KxIndexedEnum::Value<IntrinsicActionDef, IntrinsicActionID::None>;
}

namespace Kortex::GameConfig
{
	class Item;

	class IAction: public KxRTTI::Interface<IAction>
	{
		KxDecalreIID(IAction, {0x1f517242, 0xafe0, 0x41e7, {0x84, 0x6b, 0x1e, 0x67, 0x97, 0xe, 0x16, 0xf1}});

		public:
			static bool InvokeIntrinsicAction(IntrinsicActionID id, Item& item, ItemValue& value);

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

	template<class TFunc>
	class GenericAction: public IAction
	{
		private:
			TFunc m_Function;

		public:
			GenericAction(TFunc func)
				:m_Function(std::move(func))
			{
			}

		public:
			void Invoke(Item& item, ItemValue& value) override
			{
				std::invoke(m_Function, item, value);
			}
	};

	template<class TFunc>
	std::unique_ptr<IAction> MakeGenericAction(TFunc func)
	{
		return std::make_unique<GenericAction<TFunc>>(std::move(func));
	}
}
