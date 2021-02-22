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
				:wxSimplebook(&parent.GetWindow(), wxID_NONE)
			{
			}

		public:
			// IWorkspaceContainer
			wxWindow& GetWindow() override
			{
				return *this;
			}
			const wxWindow& GetWindow() const override
			{
				return *this;
			}
	};
}
