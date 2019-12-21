#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxFramework/DataView2/DataView2Fwd.h"
#include <Kx/RTTI.hpp>

namespace Kortex::GameConfig
{
	class IViewItem: public KxRTTI::Interface<IViewItem>
	{
		KxDecalreIID(IViewItem, {0x29f43e3e, 0x3375, 0x403a, {0x89, 0x5c, 0x2b, 0xf6, 0x21, 0xd3, 0x98, 0x3b}});

		public:
			virtual void OnAttachToView() {}
			virtual void OnSelect(KxDataView2::Column& column) {}
			virtual void OnActivate(KxDataView2::Column& column) {}

			virtual wxString GetViewString(ColumnID id) const = 0;
	};
}
