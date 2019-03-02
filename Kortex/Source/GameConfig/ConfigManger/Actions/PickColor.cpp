#include "stdafx.h"
#include "PickColor.h"
#include <Kortex/Application.hpp>
#include <wx/generic/colrdlgg.h>

namespace Kortex::GameConfig::Actions
{
	void PickColor::Invoke(ItemValue& value)
	{
		wxColourData colorData;
		colorData.SetChooseFull(true);
		colorData.SetChooseAlpha(true);
		colorData.SetColour(value.As<KxColor>());

		wxGenericColourDialog dialog(IApplication::GetInstance()->GetTopWindow(), &colorData);
		if (dialog.ShowModal() == wxID_OK)
		{
			value.Assign(KxColor(dialog.GetColourData().GetColour()));
		}
	}
}
