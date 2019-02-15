#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex::GameConfig::SamplingFunction
{
	class GetVideoAdapters: public ISamplingFunction
	{
		private:
			SampleValue::Vector& m_Values;

		protected:
			void DoCall();
			void OnCall(const ItemValue::Vector& arguments) override;
			
		public:
			GetVideoAdapters(SampleValue::Vector& values)
				:m_Values(values)
			{
			}
	};
}
