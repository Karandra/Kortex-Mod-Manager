#pragma once
#include "stdafx.h"
#include <Kortex/ModTagManager.hpp>
#include "Utility/KLabeledValue.h"
class KPackageCreatorController;
class KxDataViewComboBox;

namespace Kortex
{
	class IModTag;
}

class KPCInfoTagsListModel: public Kortex::ModTagManager::SelectorDisplayModelCB
{
	private:
		KPackageCreatorController* m_Controller = nullptr;

	protected:
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;

	public:
		void Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* pSzier = nullptr);
};
