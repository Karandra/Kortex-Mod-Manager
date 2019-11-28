#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectFileData.h"
#include <KxFramework/KxStdDialog.h>
class KxDataViewComboBox;

namespace Kortex::PackageDesigner::PageComponentsNS
{
	using FileDataSelectorItem = std::pair<PackageProject::FileItem*, bool>;

	class FileDataSelectorModel: public VectorModel<std::vector<FileDataSelectorItem>>
	{
		private:
		PackageProject::FileDataSection* m_FileData = nullptr;
			std::vector<FileDataSelectorItem> m_DataVector;
	
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
			FileDataSelectorItem* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			const FileDataSelectorItem* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(const KxStringVector& data, PackageProject::FileDataSection* fileData);
			KxStringVector GetSelectedItems() const;
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class FileDataSelectorComboBox: public FileDataSelectorModel
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
			void SetDataVector(KxStringVector& data, PackageProject::FileDataSection* fileData);
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class FileDataSelectorDialog: public KxStdDialog, public FileDataSelectorModel
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
			FileDataSelectorDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller);
			~FileDataSelectorDialog();
	};
}
