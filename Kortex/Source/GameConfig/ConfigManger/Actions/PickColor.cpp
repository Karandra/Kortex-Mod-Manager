#include "stdafx.h"
#include "PickColor.h"
#include "GameConfig/ConfigManger/Item.h"
#include <Kortex/Application.hpp>
#include <wx/generic/colrdlgg.h>

namespace Kortex::GameConfig::Actions
{
	void PickColor::Invoke(Item& item, ItemValue& value)
	{
		wxColourData colorData;
		colorData.SetChooseFull(true);
		colorData.SetChooseAlpha(true);
		colorData.SetColour(value.As<KxColor>());

		wxGenericColourDialog dialog(item.GetInvokingTopLevelWindow(), &colorData);
		if (dialog.ShowModal() == wxID_OK)
		{
			value.Assign(KxColor(dialog.GetColourData().GetColour()));
		}
	}
}
