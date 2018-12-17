#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "KInstallWizardRequirementsModel.h"
#include "PackageProject/KPackageProject.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageManager/KPackageManager.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "UI/KImageViewerDialog.h"
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextBoxDialog.h>

enum ColumnID
{
	Name,
	Version,
	ObjectState,
	Description,
};

void KInstallWizardRequirementsModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KInstallWizardRequirementsModel::OnActivateItem, this);

	/* Columns */
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 250);
	{
		wxString sVersionHeader = KTr("PackageCreator.PageRequirements.CurrentVersion") + '/' + KTr("PackageCreator.PageRequirements.RequiredVersion");
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(sVersionHeader, ColumnID::Version, KxDATAVIEW_CELL_INERT, 225);
	}
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("PackageCreator.PageRequirements.RequiredState"), ColumnID::ObjectState, KxDATAVIEW_CELL_INERT, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Description"), ColumnID::Description, KxDATAVIEW_CELL_INERT);
}

void KInstallWizardRequirementsModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const KPPRRequirementEntry* entry = m_DataVector[row];
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString label;
				if (entry->GetName() != entry->GetID() && entry->GetID() != Kortex::IGameInstance::GetActive()->GetGameID())
				{
					label = wxString::Format("%s (%s)", entry->GetName(), entry->GetID());
				}
				else
				{
					label = entry->GetName();
				}
				data = KxDataViewBitmapTextValue(label, GetIconByState(entry->GetOverallStatus()));
				break;
			}
			case ColumnID::Version:
			{
				wxBitmap icon = GetIconByState(entry->CheckVersion() ? wxCHK_CHECKED : wxCHK_UNCHECKED);
				if ((!entry->GetCurrentVersion().IsOK() && !entry->GetRequiredVersion().IsOK()) || !entry->GetRequiredVersion().IsOK())
				{
					data = icon;
				}
				else
				{
					wxString cv = entry->GetCurrentVersion().ToString();
					wxString rv = entry->GetRequiredVersion().ToString();
					wxString operatorSymbol = KPackageProject::OperatorToSymbolicName(entry->GetRVFunction());
					data = KxDataViewBitmapTextValue(KxString::Format("%1 %2 %3", cv, operatorSymbol, rv), icon);
				}
				break;
			}
			case ColumnID::ObjectState:
			{
				const wxString& sObject = entry->GetObject();
				KPPRObjectFunction objectFunc = entry->GetObjectFunction();
				wxBitmap icon = GetIconByState(entry->GetObjectFunctionResult());
				bool bFileFunction = objectFunc == KPPR_OBJFUNC_FILE_EXIST || objectFunc == KPPR_OBJFUNC_FILE_NOT_EXIST;

				// There's not much sense displaying required state string under this conditions
				if (bFileFunction && sObject.IsEmpty())
				{
					data = icon;
				}
				else
				{
					wxString sReqState = KTr("PackageCreator.PageRequirements.RequiredState." + KPackageProjectRequirements::ObjectFunctionToString(objectFunc));
					if (!sObject.IsEmpty())
					{
						data = KxDataViewBitmapTextValue(wxString::Format("%s: \"%s\"", sReqState, sObject), icon);
					}
					else
					{
						data = KxDataViewBitmapTextValue(sReqState, icon);
					}
				}
				break;
			}
			case ColumnID::Description:
			{
				data = entry->GetDescription();
				break;
			}
		};
	}
}
bool KInstallWizardRequirementsModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	return false;
}
bool KInstallWizardRequirementsModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return false;
}

wxIcon KInstallWizardRequirementsModel::GetIconByState(KPPReqState state) const
{
	switch (state)
	{
		case KPPReqState::False:
		{
			return KGetIcon(KIMG_CROSS_CIRCLE_FRAME);
		}
		case KPPReqState::Unknown:
		{
			return KGetIcon(KIMG_EXCLAMATION);
		}
	};
	return KGetIcon(KIMG_TICK_CIRCLE_FRAME);
}
void KInstallWizardRequirementsModel::OnActivateItem(KxDataViewEvent& event)
{
	if (event.GetColumn())
	{
		switch (event.GetColumn()->GetID())
		{
			case ColumnID::Description:
			{
				KPPRRequirementEntry* entry = GetDataEntry(event.GetItem());
				if (entry && !entry->GetDescription().IsEmpty())
				{
					KTextEditorDialog dialog(GetViewTLW());
					dialog.SetText(entry->GetDescription());
					dialog.SetEditable(false);
					dialog.ShowPreview(true);
					dialog.ShowModal();
				}
				break;
			}
		};
	}
}

void KInstallWizardRequirementsModel::SetDataVector()
{
	m_DataVector.clear();
	m_RequirementsInfo = nullptr;
	KDataViewVectorListModel::SetDataVector();
}
void KInstallWizardRequirementsModel::SetDataVector(const KPackageProjectRequirements* reqsInfo, const KxStringVector& reqIDs)
{
	m_DataVector.clear();
	m_RequirementsInfo = reqsInfo;

	for (const wxString& id: reqIDs)
	{
		KPPRRequirementsGroup* group = reqsInfo->FindGroupWithID(id);
		if (group)
		{
			for (const auto& entry: group->GetEntries())
			{
				// No need to show requirements with no object function and no required version.
				// They always be true.
				if (entry->GetObjectFunction() == KPPR_OBJFUNC_NONE && !entry->GetRequiredVersion().IsOK())
				{
					break;
				}

				m_DataVector.push_back(entry.get());
			}
		}
	}
	KDataViewVectorListModel::SetDataVector(&m_DataVector);
}
