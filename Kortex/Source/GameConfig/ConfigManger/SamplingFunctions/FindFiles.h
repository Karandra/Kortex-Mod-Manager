#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex::GameConfig::SamplingFunction
{
	class FindFiles: public ISamplingFunction
	{
		private:
			SampleValue::Vector& m_Values;

		protected:
			void DoCall(const wxString& sourcePath) const;
			void OnCall(const ItemValue::Vector& arguments) override;
			
		public:
			FindFiles(SampleValue::Vector& values)
				:m_Values(values)
			{
			}
	};
}
