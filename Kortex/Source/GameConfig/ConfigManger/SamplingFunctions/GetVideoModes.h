#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISamplingFunction.h"

namespace Kortex
{
	class IConfigManager;
}

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
			IConfigManager& m_Manager;

		protected:
			void DoCall(Component component, const wxString& format = {});
			void OnCall(const ItemValue::Vector& arguments) override;
			
		public:
			GetVideoModes(SampleValue::Vector& values, IConfigManager& manager)
				:m_Values(values), m_Manager(manager)
			{
			}
	};
}
