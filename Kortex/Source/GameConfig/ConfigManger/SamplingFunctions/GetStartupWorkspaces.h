#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex::GameConfig::SamplingFunction
{
	class GetStartupWorkspaces: public ISamplingFunction
	{
		private:
			SampleValue::Vector& m_Values;

		protected:
			void OnCall(const ItemValue::Vector& arguments) override;
			
		public:
			GetStartupWorkspaces(SampleValue::Vector& values)
				:m_Values(values)
			{
			}
	};
}
