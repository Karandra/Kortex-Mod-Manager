#pragma once
#include "stdafx.h"
#include "Common.h"
#include "Application/RTTI.h"
#include "KxFramework/DataView2/DataView2Fwd.h"

namespace Kortex::GameConfig
{
	class IViewItem: public RTTI::IInterface<IViewItem>
	{
		public:
			virtual void OnAttachToView() { }
			virtual void OnSelect(KxDataView2::Column& column) { }
			virtual void OnActivate(KxDataView2::Column& column) { }

			virtual wxString GetViewString(ColumnID id) const = 0;
	};
}
