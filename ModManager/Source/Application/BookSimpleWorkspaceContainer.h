#pragma once
#include "Framework.hpp"
#include "BookWorkspaceContainer.h"
#include <wx/simplebook.h>

namespace Kortex::Application
{
	class KORTEX_API BookSimpleWorkspaceContainer: public wxSimplebook, public kxf::RTTI::Implementation<BookSimpleWorkspaceContainer, BookWorkspaceContainer>
	{
		public:
			BookSimpleWorkspaceContainer(wxWindow* parent = nullptr)
				:wxSimplebook(parent, wxID_NONE)
			{
			}
			BookSimpleWorkspaceContainer(IWorkspaceContainer& parent)
				:wxSimplebook(parent.GetWidget().GetWxWindow(), wxID_NONE)
			{
			}

		public:
			// IWorkspaceContainer
			kxf::IWidget& GetWidget() const override
			{
				// TODO: Dirty hack for a test!!!!
				return *reinterpret_cast<kxf::IWidget*>(const_cast<BookSimpleWorkspaceContainer*>(this));
			}
	};
}
