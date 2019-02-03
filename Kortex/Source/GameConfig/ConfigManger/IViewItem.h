#pragma once
#include "stdafx.h"
#include "Common.h"
#include "Application/RTTI.h"

namespace Kortex::GameConfig
{
	class IViewItem: public RTTI::IInterface<IViewItem>
	{
		public:
			virtual wxString GetStringRepresentation(ColumnID id) const = 0;
	};
}
