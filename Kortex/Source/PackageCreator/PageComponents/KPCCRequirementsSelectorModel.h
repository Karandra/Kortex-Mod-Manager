#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::PackageDesigner
{
	using KPCCRequirementsSelectorDataElement = std::pair<KPPRRequirementsGroup*, bool>;
	using KPCCRequirementsSelectorDataArray = std::vector<KPCCRequirementsSelectorDataElement>;
}

namespace Kortex::PackageDesigner
{
	class KPCCRequirementsSelectorModel: public KPackageCreatorVectorModel<KPCCRequirementsSelectorDataArray>
	{
		private:
			KPackageProjectRequirements* m_ReqData = nullptr;
			KPCCRequirementsSelectorDataArray m_DataVector;
	
		private:
			void OnInitControl() override;
	
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
	
		public:
			const KPCCRequirementsSelectorDataElement* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			KPCCRequirementsSelectorDataElement* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(const KxStringVector& data, KPackageProjectRequirements* reqData);
			KxStringVector GetSelectedItems() const;
	};
}

namespace Kortex::PackageDesigner
{
	class KPCCRequirementsSelectorModelDialog: public KxStdDialog, public KPCCRequirementsSelectorModel
	{
		private:
			wxWindow* m_ViewPane = nullptr;
	
		private:
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
			wxWindow* GetDialogFocusCtrl() const override
			{
				return GetView();
			}
	
		public:
			KPCCRequirementsSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller);
			~KPCCRequirementsSelectorModelDialog();
	};
}
