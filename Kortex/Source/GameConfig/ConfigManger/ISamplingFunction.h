#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ItemValue.h"
#include "ItemSamples.h"

namespace Kortex::GameConfig
{
	class ISamplingFunction: public KxRTTI::Interface<ISamplingFunction>
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

	template<class TFunc>
	class GenericSamplingFunction: public ISamplingFunction
	{
		private:
			TFunc m_Function;

		protected:
			void OnCall(const ItemValue::Vector& arguments) override
			{
				std::invoke(m_Function, arguments);
			}

		public:
			GenericSamplingFunction(TFunc func)
				:m_Function(std::move(func))
			{
			}
	};

	template<class TFunc>
	std::unique_ptr<ISamplingFunction> MakeGenericSamplingFunction(TFunc func)
	{
		return std::make_unique<GenericSamplingFunction<TFunc>>(std::move(func));
	}
}
