#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ItemValue.h"
#include "ItemSamples.h"

namespace Kortex::GameConfig
{
	class ISamplingFunction: RTTI::IInterface<ISamplingFunction>
	{
		protected:
			virtual void OnCall(const ItemValue::Vector& arguments) = 0;

		public:
			ISamplingFunction(const ISamplingFunction&) = delete;
			ISamplingFunction(ISamplingFunction&&) = delete;

			ISamplingFunction() = default;
			virtual ~ISamplingFunction() = default;

		public:
			void Invoke(const ItemValue::Vector& arguments)
			{
				OnCall(arguments);
			}
	
		public:
			ISamplingFunction& operator=(const ISamplingFunction&) = delete;
			ISamplingFunction& operator=(ISamplingFunction&&) = delete;
	};
}
