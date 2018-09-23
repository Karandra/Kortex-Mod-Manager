#pragma once
#include "stdafx.h"
#include "ModManager/KModTagsSelector.h"
#include "KLabeledValue.h"
class KPackageCreatorController;
class KModTag;
class KxDataViewComboBox;

class KPCInfoTagsListModel: public KModTagsSelectorCB
{
	private:
		KPackageCreatorController* m_Controller = NULL;

	protected:
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;

	public:
		void Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* pSzier = NULL);
};
