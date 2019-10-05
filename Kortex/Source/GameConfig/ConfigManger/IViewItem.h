#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxFramework/DataView2/DataView2Fwd.h"
#include <Kx/RTTI.hpp>

namespace Kortex::GameConfig
{
	class IViewItem: public KxRTTI::Interface<IViewItem>
	{
		public:
			virtual void OnAttachToView() {}
			virtual void OnSelect(KxDataView2::Column& column) {}
			virtual void OnActivate(KxDataView2::Column& column) {}

			virtual wxString GetViewString(ColumnID id) const = 0;
	};
}
