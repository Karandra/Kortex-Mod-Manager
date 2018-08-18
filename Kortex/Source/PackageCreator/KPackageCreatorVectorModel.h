#pragma once
#include "stdafx.h"
#include "KPackageCreatorListModel.h"

template<class T> class KPackageCreatorVectorModel: public KDataViewVectorListModel<T, KPackageCreatorListModel>
{
	private:
		virtual void OnSetDataVectorInternal() override
		{
			KDataViewVectorListModel::OnSetDataVectorInternal();
		}
};
