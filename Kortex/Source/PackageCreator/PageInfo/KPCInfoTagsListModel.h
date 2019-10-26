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
	class KPackageCreatorController;
}

namespace Kortex::PackageDesigner
{
	class KPCInfoTagsListModel: public Kortex::ModTagManager::SelectorDisplayModelCB
	{
		private:
			KPackageCreatorController* m_Controller = nullptr;
			
		protected:
			bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
			
		public:
			void Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* sizer = nullptr);
	};
}
