#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectFileData.h"
#include "KProgramOptions.h"
#include <KxFramework/KxStdDialog.h>

using KPCCFileDataSelectorDataElement = std::pair<KPPFFileEntry*, bool>;
using KPCCFileDataSelectorDataArray = std::vector<KPCCFileDataSelectorDataElement>;
class KPCCFileDataSelectorModel: public KPackageCreatorVectorModel<KPCCFileDataSelectorDataArray>
{
	private:
		KPackageProjectFileData* m_FileData = NULL;
		KPCCFileDataSelectorDataArray m_DataVector;

	private:
		virtual void OnInitControl() override;

		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
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
			return NULL;
		}
		const KPCCFileDataSelectorDataElement* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return &GetDataVector()->at(index);
			}
			return NULL;
		}

		void SetDataVector();
		void SetDataVector(const KxStringVector& data, KPackageProjectFileData* fileData);
		KxStringVector GetSelectedItems() const;
};

//////////////////////////////////////////////////////////////////////////
class KxDataViewComboBox;
class KPCCFileDataSelectorModelCB: public KPCCFileDataSelectorModel
{
	private:
		KxDataViewComboBox* m_ComboView = NULL;
		KxStringVector* m_RequiredFiles = NULL;

	private:
		virtual KxDataViewCtrl* OnCreateDataView(wxWindow* window) override;
		virtual wxWindow* OnGetDataViewWindow() override;
		virtual void OnSetDataVector() override;
		void OnGetStringValue(KxDataViewEvent& event);

	public:
		void SetDataVector();
		void SetDataVector(KxStringVector& data, KPackageProjectFileData* fileData);
};

//////////////////////////////////////////////////////////////////////////
class KPCCFileDataSelectorModelDialog: public KxStdDialog, public KPCCFileDataSelectorModel
{
	private:
		wxWindow* m_ViewPane = NULL;

		KProgramOptionUI m_WindowOptions;
		KProgramOptionUI m_ViewOptions;

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
		virtual ~KPCCFileDataSelectorModelDialog();
};
