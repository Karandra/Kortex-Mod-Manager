#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex::GameConfig::SamplingFunction
{
	class GetVideoModes: public ISamplingFunction
	{
		public:
			enum Component
			{
				None = 0,
				Width = 1 << 0,
				Height = 1 << 1,
				Both = Width|Height
			};

		private:
			SampleValue::Vector& m_Values;

		protected:
			void DoCall(Component component);
			void OnCall(const ItemValue::Vector& arguments) override;
			
		public:
			GetVideoModes(SampleValue::Vector& values)
				:m_Values(values)
			{
			}
	};
}
