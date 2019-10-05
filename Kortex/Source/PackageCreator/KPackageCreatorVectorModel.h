#pragma once
#include "stdafx.h"
#include "KPackageCreatorListModel.h"
#include "KxFramework/KxDataViewModelExBase.h"

namespace Kortex::PackageDesigner
{
	template<class T>
	class KPackageCreatorVectorModel: public KxDataViewVectorListModelEx<T, KPackageCreatorListModel>
	{
		private:
			void OnSetDataVectorInternal() override
			{
				KxDataViewVectorListModelEx::OnSetDataVectorInternal();
			}
	};
}
