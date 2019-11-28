#pragma once
#include "stdafx.h"
#include <Kortex/ModTagManager.hpp>
#include "Utility/KLabeledValue.h"
class KxDataViewComboBox;

namespace Kortex
{
	class IModTag;
}
namespace Kortex::PackageDesigner
{
	class WorkspaceDocument;
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	class TagsListModel: public Kortex::ModTagManager::SelectorDisplayModelCB
	{
		private:
			WorkspaceDocument* m_Controller = nullptr;
			
		protected:
			bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
			
		public:
			void Create(WorkspaceDocument* controller, wxWindow* window, wxSizer* sizer = nullptr);
	};
}
