#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectFileData.h"
#include <KxFramework/KxStdDialog.h>
class KxDataViewComboBox;

namespace Kortex::PackageDesigner
{
	using KPCCFileDataSelectorDataElement = std::pair<KPPFFileEntry*, bool>;
	using KPCCFileDataSelectorDataArray = std::vector<KPCCFileDataSelectorDataElement>;
}

namespace Kortex::PackageDesigner
{
	class KPCCFileDataSelectorModel: public KPackageCreatorVectorModel<KPCCFileDataSelectorDataArray>
	{
		private:
			KPackageProjectFileData* m_FileData = nullptr;
			KPCCFileDataSelectorDataArray m_DataVector;
	
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
			KPCCFileDataSelectorDataElement* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			const KPCCFileDataSelectorDataElement* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(const KxStringVector& data, KPackageProjectFileData* fileData);
			KxStringVector GetSelectedItems() const;
	};
}

namespace Kortex::PackageDesigner
{
	class KPCCFileDataSelectorModelCB: public KPCCFileDataSelectorModel
	{
		private:
			KxDataViewComboBox* m_ComboView = nullptr;
			KxStringVector* m_RequiredFiles = nullptr;
	
		private:
			KxDataViewCtrl* OnCreateDataView(wxWindow* window) override;
			wxWindow* OnGetDataViewWindow() override;
			void OnSetDataVector() override;
			void OnGetStringValue(KxDataViewEvent& event);
	
		public:
			void SetDataVector();
			void SetDataVector(KxStringVector& data, KPackageProjectFileData* fileData);
	};
}

namespace Kortex::PackageDesigner
{
	class KPCCFileDataSelectorModelDialog: public KxStdDialog, public KPCCFileDataSelectorModel
	{
		private:
			wxWindow* m_ViewPane = nullptr;
	
			//KProgramOptionAI m_WindowOptions;
			//KProgramOptionAI m_ViewOptions;
	
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
			KPCCFileDataSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller);
			~KPCCFileDataSelectorModelDialog();
	};
}
