#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex::GameConfig
{
	template<class TFunctor, size_t requireArgsCount = std::numeric_limits<size_t>::max()>
	class Generic: public RTTI::IExtendInterface<Generic, ISamplingFunction>
	{
		private:
			TFunctor m_Functor;
			ItemValue::Vector& m_Values;

		public:
			Generic(T&& func, ItemValue::Vector& values)
				:m_Functor(std::move(func)), m_Values(values)
			{
			}

		public:
			void operator()(const ItemValue::Vector& arguments) override
			{
				if (requireArgsCount == std::numeric_limits<size_t>::max() || arguments.size() >= requireArgsCount)
				{
					std::invoke(m_Functor, m_Values, arguments);
				}
			}
	};
}
