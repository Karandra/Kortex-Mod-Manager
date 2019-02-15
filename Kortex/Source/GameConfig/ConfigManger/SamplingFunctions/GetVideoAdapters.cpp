#include "stdafx.h"
#include "GetVideoAdapters.h"
#include <KxFramework/KxSystemSettings.h>

namespace Kortex::GameConfig::SamplingFunction
{
	void GetVideoAdapters::DoCall()
	{
		for (const DISPLAY_DEVICE& adapter: KxSystemSettings::EnumVideoAdapters())
		{
			// DeviceString field is wchar_t[] array so wrap it into wxString,
			// otherwise wxAny will store pointer to the array and not copy its content as wxString.
			m_Values.emplace_back(wxString(adapter.DeviceString));
		}
	}
	void GetVideoAdapters::OnCall(const ItemValue::Vector& arguments)
	{
		DoCall();
	}
}
