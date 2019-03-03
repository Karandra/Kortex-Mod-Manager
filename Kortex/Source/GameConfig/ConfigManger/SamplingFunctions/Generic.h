#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex::GameConfig
{
	template<class TFunctor, size_t requiredArgsCount = std::numeric_limits<size_t>::max()>
	class Generic: public ISamplingFunction
	{
		private:
			ItemValue::Vector& m_Values;
			TFunctor m_Functor;

		protected:
			void OnCall(const ItemValue::Vector& arguments) override
		{
			if (requiredArgsCount == std::numeric_limits<size_t>::max() || arguments.size() >= requiredArgsCount)
			{
				std::invoke(m_Functor, m_Values, arguments);
			}
		}

		public:
			Generic(ItemValue::Vector& values, TFunctor&& func)
				:m_Values(values), m_Functor(std::move(func))
			{
			}
	};
}
