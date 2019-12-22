#include "stdafx.h"
#include "BethesdaBasicSave.h"

namespace Kortex::SaveManager::BethesdaSave
{
	void BethesdaBasicSave::SortBasicInfo()
	{
		std::sort(m_BasicInfo.begin(), m_BasicInfo.end(), [](const SaveInfoPair& left, const SaveInfoPair& right)
		{
			return left.Order() < right.Order();
		});
	}

	wxString BethesdaBasicSave::GetDisplayName() const
	{
		wxString displayName;
		auto AddPart = [&displayName](const SaveInfoPair& value)
		{
			if (value.ShouldDisplay())
			{
				if (!displayName.IsEmpty() && value.HasValue())
				{
					displayName += wxS(", ");
				}

				if (value.HasValue())
				{
					if (value.ShouldDisplayLabel() && value.HasLabel())
					{
						displayName += KxString::Format(wxS("%1: %2"), value.GetRawLabel(), value.GetValue());
					}
					else
					{
						displayName += value.GetValue();
					}
				}
			}
		};

		for (const SaveInfoPair& value: m_BasicInfo)
		{
			AddPart(value);
		}
		return displayName.IsEmpty() ? GetFileItem().GetName() : displayName;
	}
}
