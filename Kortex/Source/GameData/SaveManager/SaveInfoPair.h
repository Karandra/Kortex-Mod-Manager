#pragma once
#include "stdafx.h"
#include "Utility/LabeledValue.h"

namespace Kortex::SaveManager
{
	class SaveInfoPair: public Utility::LabeledValue
	{
		public:
			using Vector = std::vector<SaveInfoPair>;

		private:
			int m_Order = std::numeric_limits<int>::max();
			bool m_Display = false;
			bool m_DisplayLabel = false;

		public:
			SaveInfoPair() = default;
			SaveInfoPair(const wxString& value, const wxString& label = {})
				:Utility::LabeledValue(value, label)
			{
			}

		public:
			SaveInfoPair& Order(int value)
			{
				m_Order = value;
				return *this;
			}
			int Order() const
			{
				return m_Order;
			}
			
			SaveInfoPair& Display(bool value = true)
			{
				m_Display = value;
				return *this;
			}
			SaveInfoPair& DisplayLabel(bool value = true)
			{
				m_DisplayLabel = value;
				return *this;
			}

			bool ShouldDisplay() const
			{
				return m_Display;
			}
			bool ShouldDisplayLabel() const
			{
				return m_DisplayLabel;
			}
	};
}
