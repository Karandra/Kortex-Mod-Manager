#include "stdafx.h"
#include "KCMController.h"
#include "KCMIDataProvider.h"
#include "UI/KWorkspace.h"
#include "KConfigManager.h"
#include "Profile/KProfile.h"
#include "Events/KLogEvent.h"
#include "KApp.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxListBoxDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#pragma warning (disable: 4302)
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)

void KCMController::CreateContextMenu()
{
	m_ContextMenu = new KxMenu();

	m_ContextMenu_EditValue = m_ContextMenu->Add(new KxMenuItem(T("ConfigManager.EditValue.ModifyValue")));
	m_ContextMenu_EditValue->SetBitmap(KGetBitmap(KIMG_GEAR_PENCIL));

	m_ContextMenu_CreateValue = m_ContextMenu->Add(new KxMenuItem(T("ConfigManager.EditValue.CreateValue")));
	m_ContextMenu_CreateValue->SetBitmap(KGetBitmap(KIMG_GEAR_PLUS));

	m_ContextMenu->AddSeparator();

	m_ContextMenu_RemoveValue = m_ContextMenu->Add(new KxMenuItem(T("ConfigManager.EditValue.RemoveValue")));
	m_ContextMenu_RemoveValue->SetBitmap(KGetBitmap(KIMG_GEAR_MINUS));

	m_ContextMenu_RemovePath = m_ContextMenu->Add(new KxMenuItem(T("ConfigManager.EditValue.RemovePath")));
	m_ContextMenu_RemovePath->SetBitmap(KGetBitmap(KIMG_CATEGORY_ITEM_MINUS));

	m_ContextMenu->AddSeparator();
	{
		KxMenuItem* item = m_ContextMenu->Add(new KxMenuItem(T(KxID_REFRESH)));
		item->SetBitmap(KGetBitmap(KIMG_ARROW_CIRCLE_DOUBLE));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			GetWorkspace()->ReloadWorkspace();
		});
	}
}

KxTreeListItem KCMController::GetCategoryNode(const wxString& sCategoryPath, const KxTreeListItem& tExtraRoot, bool bLocalize)
{
	if (!sCategoryPath.IsEmpty())
	{
		// If requested category already exist return it
		if (m_CategoriesTable.count(sCategoryPath))
		{
			return m_CategoriesTable.at(sCategoryPath);
		}

		// If not, create a new one
		KxStringVector tCategoryPath = KxString::Split(sCategoryPath, m_ConfigManager->GetCategorySeparator(), false);
		if (!tCategoryPath.empty())
		{
			const auto& tCategoryNamesList = m_ConfigManager->GetCategories();
			KxTreeListItem node = tExtraRoot;
			KxStringVector names;
			for (const wxString& id: tCategoryPath)
			{
				names.push_back(id);

				// Get localization for this part of category path
				wxString label;
				if (bLocalize)
				{
					label = GetCategoryName(id);
				}
				if (label.IsEmpty())
				{
					label = id;
				}

				// Add created part to table or get it to use in next iteration
				wxString path = KxString::Join(names, KConfigManager::GetCategorySeparator());
				if (!m_CategoriesTable.count(path))
				{
					node = (node.IsOK() ? node : m_TreeRoot).Add(label);
					m_CategoriesTable.emplace(std::make_pair(path, node));
				}
				else
				{
					node = m_CategoriesTable.at(path);
				}
			}

			// All nodes on given path has been constructed
			// Return last node
			return node;
		}
	}
	return m_UnknownItemsRoot;
}
void KCMController::AddSampleValues(KxComboBoxDialog* dialog, KCMConfigEntryStd* configEntry)
{
	wxItemContainer* pItemsContainer = dynamic_cast<wxItemContainer*>(dialog->GetDialogMainCtrl());
	if (pItemsContainer)
	{
		for (const auto& item: configEntry->GetSampleValues())
		{
			pItemsContainer->Append(configEntry->OnDisplaySampleValue(item), (void*)&item);
		}
	}
}
KxDialog* KCMController::CreateDialog(KCMConfigEntryStd* configEntry)
{
	KxDialog* dialog = NULL;
	if (KCMConfigEntryFileBrowse* pFB = configEntry->ToFileBrowseEntry())
	{
		dialog = OnCreateFileBrowseDialog(pFB);
	}
	else if (KCMConfigEntryArray* pArray = configEntry->ToArrayEntry())
	{
		dialog = OnCreateListBoxDialog(pArray);
	}
	else
	{
		if (configEntry->HasSampleValues())
		{
			dialog = OnCreateComboBoxDialog(configEntry);
		}
		else
		{
			dialog = OnCreateTextBoxDialog(configEntry);
		}
	}
	return dialog;
}
wxString KCMController::GetDialogValue(KxDialog* dialog, KCMConfigEntryStd* configEntry)
{
	if (KCMConfigEntryFileBrowse* pFB =  configEntry->ToFileBrowseEntry())
	{
		return static_cast<KxFileBrowseDialog*>(dialog)->GetResult();
	}
	else
	{
		if (configEntry->HasSampleValues())
		{
			KxComboBoxDialog* pCBDialog = static_cast<KxComboBoxDialog*>(dialog);
			if (!configEntry->IsEditable())
			{
				int index = pCBDialog->GetSelection();
				if (index != -1)
				{
					const KCMSampleValue* pValue = static_cast<const KCMSampleValue*>(pCBDialog->GetComboBox()->GetClientData(index));
					if (pValue)
					{
						return pValue->GetValue();
					}
				}
			}
			return pCBDialog->GetValue();
		}
		else
		{
			return static_cast<KxTextBoxDialog*>(dialog)->GetValue();
		}
	}
}
void KCMController::SetDialogValidator(KxDialog* dialog, KCMConfigEntryStd* configEntry)
{
	if (!configEntry->ToFileBrowseEntry())
	{
		KxStdDialog* pStdDialog = static_cast<KxStdDialog*>(dialog);
		if (configEntry->IsEditable())
		{
			KCMDataType type = configEntry->GetType();
			if (KConfigManager::IsSignedIntType(type))
			{
				wxIntegerValidator<int64_t> tValidator;
				int64_t nMin;
				int64_t max;

				if (configEntry->GetMinValue().second)
				{
					nMin = std::get<0>(configEntry->GetMinValue().first);;
				}
				else
				{
					nMin = KConfigManager::GetMinMaxSignedValue(type).first;
				}
				if (configEntry->GetMaxValue().second)
				{
					max = std::get<0>(configEntry->GetMaxValue().first);
				}
				else
				{
					max = KConfigManager::GetMinMaxSignedValue(type).second;
				}
				tValidator.SetMin(nMin);
				tValidator.SetMax(max);

				pStdDialog->GetDialogMainCtrl()->SetValidator(tValidator);
				pStdDialog->SetLabel(wxString::Format("%s: [%s; %s]", T("ConfigManager.EditValue.ValuesRange"), configEntry->GetFormatter()(nMin), configEntry->GetFormatter()(max)));
			}
			else if (KConfigManager::IsUnsignedIntType(type))
			{
				wxIntegerValidator<uint64_t> tValidator;
				uint64_t nMin;
				uint64_t max;

				if (configEntry->GetMinValue().second)
				{
					nMin = std::get<1>(configEntry->GetMinValue().first);
				}
				else
				{
					nMin = KConfigManager::GetMinMaxUnsignedValue(type).first;
				}
				if (configEntry->GetMaxValue().second)
				{
					max = std::get<1>(configEntry->GetMaxValue().first);
				}
				else
				{
					max = KConfigManager::GetMinMaxUnsignedValue(type).second;
				}
				tValidator.SetMin(nMin);
				tValidator.SetMax(max);

				pStdDialog->GetDialogMainCtrl()->SetValidator(tValidator);
				pStdDialog->SetLabel(wxString::Format("%s: [%s; %s]", T("ConfigManager.EditValue.ValuesRange"), configEntry->GetFormatter()(nMin), configEntry->GetFormatter()(max)));
			}
			else if (KConfigManager::IsFloatType(type))
			{
				wxFloatingPointValidator<double> tValidator;
				int nPrecision = configEntry->GetFormatter().GetFloatPrecision();
				double nMin;
				double max;

				if (configEntry->GetMinValue().second)
				{
					nMin = std::get<2>(configEntry->GetMinValue().first);
				}
				else
				{
					nMin = KConfigManager::GetMinMaxFloatValue(type).first;
				}
				if (configEntry->GetMaxValue().second)
				{
					max = std::get<2>(configEntry->GetMaxValue().first);
				}
				else
				{
					max = KConfigManager::GetMinMaxFloatValue(type).second;
				}
				tValidator.SetMin(nMin);
				tValidator.SetMax(max);
				tValidator.SetPrecision(nPrecision);

				pStdDialog->GetDialogMainCtrl()->SetValidator(tValidator);
				pStdDialog->SetLabel(wxString::Format("%s: [%g; %g]", T("ConfigManager.EditValue.ValuesRange"), nMin, max));
			}
		}
	}
}
void KCMController::ConfigureForVirtualKey(KxStdDialog* dialog, KCMConfigEntryVK* pVKEntry)
{
	dialog->SetLabel(T("ConfigManager.EditValue.EnterVirtualKey"));
	dialog->SetClientData((void*)pVKEntry->GetDataKeyCode());
	wxComboBox* pComboBox = dynamic_cast<wxComboBox*>(dialog->GetDialogMainCtrl());
	wxTextEntry* pTextEntry = NULL;
	if (!pComboBox)
	{
		pTextEntry = dynamic_cast<wxTextEntry*>(dialog->GetDialogMainCtrl());
		if (pTextEntry)
		{
			pTextEntry->SetEditable(false);
		}
	}
	else
	{
		pComboBox->SetEditable(false);
		pComboBox->Bind(wxEVT_COMBOBOX, [pComboBox, dialog](wxCommandEvent& event)
		{
			if (event.GetSelection() != -1)
			{
				unsigned long value = WXK_NONE;
				const KCMSampleValue* pValue = static_cast<const KCMSampleValue*>(pComboBox->GetClientData(event.GetSelection()));
				if (pValue && pValue->GetValue().ToCULong(&value))
				{
					dialog->SetClientData((void*)value);
				}
			}
		});
	}

	auto OnKeyEvent = [this, dialog, pVKEntry, pTextEntry, pComboBox](wxKeyEvent& event)
	{
		dialog->SetClientData((void*)event.GetRawKeyCode());

		if (pTextEntry)
		{
			pTextEntry->SetValue(std::to_string(event.GetRawKeyCode()));
		}
		else if (pComboBox)
		{
			wxString value = std::to_string(event.GetRawKeyCode());
			int index = pVKEntry->FindDataInSamples(value);
			if (index != wxNOT_FOUND)
			{
				pComboBox->SetSelection(index);
			}
			else
			{
				pComboBox->SetValue(value);
			}
		}
	};
	dialog->Bind(wxEVT_CHAR_HOOK, OnKeyEvent);
	dialog->GetDialogMainCtrl()->Bind(wxEVT_CHAR_HOOK, OnKeyEvent);
	dialog->GetDialogMainCtrl()->Bind(wxEVT_KEY_DOWN, OnKeyEvent);
}
bool KCMController::AddToHash(KCMConfigEntryPath* entry)
{
	if (KCMConfigEntryDV* pDV = entry->ToDVEntry())
	{
		auto t1 = m_AddedItems.emplace(pDV->GetFullPathFor(pDV->GetName1()));
		auto t2 = m_AddedItems.emplace(pDV->GetFullPathFor(pDV->GetName2()));
		return t1.second || t2.second;
	}
	else if (KCMConfigEntryStd* pStd = entry->ToStdEntry())
	{
		return m_AddedItems.emplace(pStd->GetFullPath()).second;
	}
	else if (KCMConfigEntryPath* pPath = entry->ToPathEntry())
	{
		return m_AddedItems.emplace(pPath->GetFullPath()).second;
	}
	else
	{
		return m_AddedItems.emplace(entry->GetPath()).second;
	}
}
bool KCMController::IsInHash(KCMConfigEntryPath* entry)
{
	if (KCMConfigEntryDV* pDV = entry->ToDVEntry())
	{
		return m_AddedItems.count(pDV->GetFullPathFor(pDV->GetName1())) && m_AddedItems.count(pDV->GetFullPathFor(pDV->GetName2()));
	}
	else if (KCMConfigEntryStd* pStd = entry->ToStdEntry())
	{
		return m_AddedItems.count(pStd->GetFullPath());
	}
	else if (KCMConfigEntryPath* pPath = entry->ToPathEntry())
	{
		return m_AddedItems.count(pPath->GetFullPath());
	}
	else
	{
		return m_AddedItems.count(entry->GetPath());
	}
}
void KCMController::UpdateItemImages(KCMConfigEntryBase* entry, KImageEnum image, bool recursive)
{
	auto SetImage = [image](KxTreeListItem& item)
	{
		item.SetImage(image, image);
	};

	KxTreeListItem item = entry->GetViewNode();
	if (item.IsOK())
	{
		if (recursive)
		{
			while (item.IsOK())
			{
				SetImage(item);
				item = item.GetParent();
			}
		}
		else
		{
			SetImage(item);
		}
	}
}
void KCMController::DiscardChangesHelper()
{
	for (auto& v: m_ModifiedEntries)
	{
		if (v.second == KCMCC_CREATE_ENTRY)
		{
			delete v.first;
		}
	}
	m_ModifiedEntries.clear();
}

const wxString& KCMController::GetCategoryName(const wxString& sCategoryID) const
{
	static const wxString ms_EmptyString = wxEmptyString;

	const auto& tCategoryNamesList = m_ConfigManager->GetCategories();
	if (tCategoryNamesList.count(sCategoryID))
	{
		return tCategoryNamesList.at(sCategoryID);
	}
	return ms_EmptyString;
}
KCMConfigEntryRef* KCMController::GetEntryFromItem(const KxTreeListItem& item) const
{
	wxClientData* data = item.GetData();
	if (data)
	{
		return static_cast<KCMConfigEntryRef*>(data);
	}
	return NULL;
}

KxComboBoxDialog* KCMController::OnCreateComboBoxDialog(KCMConfigEntryStd* configEntry)
{
	int style = KxComboBoxDialog::DefaultStyle;
	if (configEntry->IsEditable())
	{
		style &= ~KxCBD_READONLY;
	}
	auto dialog = new KxComboBoxDialog(GetParentWindow(), KxID_NONE, configEntry->GetLabel(), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, style);
	static_cast<KxComboBox*>(dialog->GetDialogMainCtrl())->SetVisibleItemsCount(20);

	AddSampleValues(dialog, configEntry);
	int index = configEntry->FindDataInSamples();
	if (index != wxNOT_FOUND)
	{
		dialog->SetSelection(index);
	}
	else if (configEntry->IsEditable())
	{
		dialog->SetValue(configEntry->GetData(true));
	}
	else
	{
		dialog->SetSelection(0);
	}
	return dialog;
}
KxTextBoxDialog* KCMController::OnCreateTextBoxDialog(KCMConfigEntryStd* configEntry)
{
	int style = KxTextBoxDialog::DefaultStyle;
	if (!configEntry->IsEditable())
	{
		style |= KxTBD_READONLY;
	}
	if (KConfigManager::IsStringType(configEntry->GetType()) || configEntry->GetType() == KCMDT_UNKNOWN)
	{
		style |= KxTBD_MULTILINE;
	}

	auto dialog = new KxTextBoxDialog(GetParentWindow(), KxID_NONE, configEntry->GetLabel(), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, style);
	dialog->SetValue(configEntry->GetData(true));
	return dialog;
}
KxListBoxDialog* KCMController::OnCreateListBoxDialog(KCMConfigEntryArray* configEntry)
{
	auto dialog = new KxListBoxDialog(GetParentWindow(), KxID_NONE, configEntry->GetLabel(), wxDefaultPosition, wxSize(600, 550), KxBTN_OK|KxBTN_CANCEL);
	dialog->SetCheckList(true);
	
	// Add all items
	for (const auto& v: configEntry->GetSampleValues())
	{
		dialog->AddItem(configEntry->OnDisplaySampleValue(v));
	}

	// Check items in data
	for (const wxString& s: configEntry->GetDataArray())
	{
		int index = configEntry->FindDataInSamples(s);
		if (index != wxNOT_FOUND)
		{
			dialog->SetItemChecked(index, true);
		}
	}
	return dialog;
}
KxFileBrowseDialog* KCMController::OnCreateFileBrowseDialog(KCMConfigEntryFileBrowse* configEntry)
{
	auto dialog = new KxFileBrowseDialog(GetParentWindow(), KxID_NONE, configEntry->IsFolder() ? KxFBD_OPEN_FOLDER : KxFBD_OPEN, configEntry->GetLabel());
	if (configEntry->IsFolder())
	{
		dialog->SetFolder(configEntry->GetData(false));
	}
	else
	{
		dialog->SetFolder(configEntry->GetData(false).BeforeFirst('\\'));
	}
	return dialog;
}
void KCMController::OnConfigureDialog(KxDialog* dialog, KCMConfigEntryStd* configEntry)
{
}
wxWindow* KCMController::GetParentWindow()
{
	return KApp::Get().GetTopWindow();
}

KxTreeListItem KCMController::CreateUnknownItemsRoot()
{
	return m_TreeRoot;
}
KxStringVector KCMController::OnFormatEntryToView(KCMConfigEntryPath* pathEntry)
{
	return KxStringVector({pathEntry->GetFullPath()});
}
KxStringVector KCMController::OnFormatEntryToView(KCMConfigEntryStd* stdEntry)
{
	return KxStringVector({stdEntry->GetFullPath(), stdEntry->GetLabel(), stdEntry->GetDisplayTypeName(), stdEntry->GetDisplayData()});
}
void KCMController::OnEditEntry(KCMConfigEntryStd* configEntry)
{
	wxWindowID nMsgRet = KxID_CANCEL;
	if (KConfigManager::IsBoolType(configEntry->GetType()))
	{
		configEntry->SetDataBool(!configEntry->GetDataBool());
		nMsgRet = KxID_OK;
	}
	else
	{
		KxDialog* dialog = CreateDialog(configEntry);
		if (dialog)
		{
			OnConfigureDialog(dialog, configEntry);
			if (KCMConfigEntryVK* pVK = configEntry->ToVKEntry())
			{
				ConfigureForVirtualKey(static_cast<KxStdDialog*>(dialog), pVK);
			}
			else
			{
				SetDialogValidator(dialog, configEntry);
			}

			nMsgRet = dialog->ShowModal();
			if (nMsgRet == KxID_OK)
			{
				if (KCMConfigEntryVK* pVK = configEntry->ToVKEntry())
				{
					wxUint32 nKeyCode = (wxUint32)dialog->GetClientData();
					if (nKeyCode != pVK->GetDataKeyCode())
					{
						pVK->SetDataKeyCode((wxKeyCode)nKeyCode);
					}
					else
					{
						nMsgRet = KxID_IGNORE;
					}
				}
				else if (KCMConfigEntryArray* pArray = configEntry->ToArrayEntry())
				{
					KxStringVector outList;

					KxListBoxDialog* pListBoxDialog = static_cast<KxListBoxDialog*>(dialog);
					KxIntVector tCheckedItems = pListBoxDialog->GetCheckedItems();
					for (int index : tCheckedItems)
					{
						outList.emplace_back(pArray->GetSampleValues()[index].GetValue());
					}
					pArray->SetDataArray(outList);
				}
				else
				{
					wxString value = GetDialogValue(dialog, configEntry);
					if (value != configEntry->GetData(false))
					{
						configEntry->SetData(value, false);
					}
					else
					{
						nMsgRet = KxID_IGNORE;
					}
				}
			}
			dialog->Destroy();
		}
	}

	if (nMsgRet == KxID_OK)
	{
		configEntry->GetViewNode().SetLabel(configEntry->GetDisplayData(), GetValueDataColumn());
		OnAddModifiedEntry(configEntry, KCMCC_MODIFY_ENTRY);
	}
}
void KCMController::OnCreateEntry(KCMConfigEntryBase* entry)
{
	KCMCAddEntryDialog dialog(this, entry);
	dialog.ShowModal();
}
void KCMController::OnRemoveEntry(KCMConfigEntryStd* configEntry, KCMConfigEntryRef* entryRef)
{
	OnAddModifiedEntry(configEntry, KCMCC_REMOVE_ENTRY);
}
void KCMController::OnRemovePath(KCMConfigEntryPath* configEntry, KCMConfigEntryRef* entryRef)
{
	OnAddModifiedEntry(configEntry, KCMCC_REMOVE_PATH);
	configEntry->GetViewNode().RemoveChildren();
	entryRef->SetDeleted();
}
void KCMController::OnAddModifiedEntry(KCMConfigEntryBase* entry, KCMControllerCommands nCommand)
{
	m_View->Freeze();
	m_ModifiedEntries.push_back(std::make_pair(entry, nCommand));
	
	KImageEnum nCommandImage = KIMG_QUESTION_FRAME;
	switch (nCommand)
	{
		case KCMCC_CREATE_ENTRY:
		{
			nCommandImage = KIMG_PLUS_SMALL;
			break;
		}
		case KCMCC_MODIFY_ENTRY:
		{
			nCommandImage = KIMG_PENCIL_SMALL;
			break;
		}
		case KCMCC_REMOVE_ENTRY:
		{
			nCommandImage = KIMG_MINUS_SMALL;
			break;
		}
		case KCMCC_REMOVE_PATH:
		{
			nCommandImage = KIMG_CATEGORY_ITEM_MINUS;
			break;
		}
	};
	UpdateItemImages(entry, KIMG_PENCIL_SMALL, true);
	UpdateItemImages(entry, nCommandImage, false);
	m_View->Thaw();

	wxNotifyEvent event(KEVT_CONTROLLER_CHNAGED);
	ProcessEvent(event);
}
void KCMController::OnActivateView(wxTreeListEvent& event)
{
	if (IsEditable())
	{
		KCMConfigEntryRef* entryRef = GetEntryFromItem(KxTreeListItem(*m_View, event.GetItem()));
		if (entryRef)
		{
			KCMConfigEntryBase* entry = entryRef->GetEntry();
			if (entry && entry->ToStdEntry())
			{
				OnEditEntry(entry->ToStdEntry());
			}
		}
	}
}
void KCMController::OnSelectView(wxTreeListEvent& event)
{
}
void KCMController::OnContextMenu(wxTreeListEvent& event)
{
	m_ContextMenu_EditValue->Enable(false);
	m_ContextMenu_CreateValue->Enable(false);
	m_ContextMenu_RemoveValue->Enable(false);
	m_ContextMenu_RemovePath->Enable(false);

	KxTreeListItem item(*m_View, event.GetItem());
	if (item.IsOK())
	{
		KCMConfigEntryRef* entryRef = GetEntryFromItem(item);
		KCMConfigEntryBase* entry = entryRef ? entryRef->GetEntry() : NULL;
		if (entry)
		{
			bool bEditable = IsEditable();
			bool bPath = entry->ToPathEntry();
			bool bStd = entry->ToStdEntry();

			m_ContextMenu_EditValue->Enable(bEditable && bStd);
			m_ContextMenu_CreateValue->Enable(bEditable && IsCreateEntryAllowed() && bPath);
			m_ContextMenu_RemoveValue->Enable(bEditable && IsRemoveEntryAllowed() && bStd);
			m_ContextMenu_RemovePath->Enable(bEditable && IsRemovePathAllowed() && (bPath && !bStd));
		}

		wxWindowID id = m_ContextMenu->Show(m_View);
		if (entry)
		{
			if (KCMConfigEntryStd* pStd = entry->ToStdEntry())
			{
				if (id == m_ContextMenu_EditValue->GetId())
				{
					OnEditEntry(pStd);
				}
				else if (id == m_ContextMenu_RemoveValue->GetId())
				{
					OnRemoveEntry(pStd, entryRef);
				}
			}
			else if (KCMConfigEntryPath* pPath = entry->ToPathEntry())
			{
				if (id == m_ContextMenu_RemovePath->GetId())
				{
					OnRemovePath(pPath, entryRef);
				}
			}
			if (id == m_ContextMenu_CreateValue->GetId())
			{
				OnCreateEntry(entry);
			}
		}
	}
}

KCMController::KCMController(KWorkspace* workspace, KConfigManager* pConfigManager, KxTreeList* view)
	:KWorkspaceController(workspace), m_ConfigManager(pConfigManager), m_View(view)
{
	CreateContextMenu();
	m_View->SetImageList(const_cast<KxImageList*>(KGetImageList()));
	
	m_View->Bind(KxEVT_TREELIST_ITEM_ACTIVATED, &KCMController::OnActivateView, this);
	m_View->Bind(KxEVT_TREELIST_SELECTION_CHANGED, &KCMController::OnSelectView, this);
	m_View->Bind(wxEVT_TREELIST_ITEM_CONTEXT_MENU, &KCMController::OnContextMenu, this);
}
KCMController::~KCMController()
{
	DiscardChangesHelper();
	delete m_ContextMenu;
}

bool KCMController::IsOK() const
{
	return KWorkspaceController::IsOK() && m_ConfigManager != NULL && m_View != NULL;
}

void KCMController::ResetView()
{
	m_View->ClearItems();
	m_CategoriesTable.clear();
	m_ModifiedEntries.clear();
	m_AddedItems.clear();
	DiscardChanges();

	// Save root item
	m_TreeRoot = m_View->GetRoot();

	// Add 'unknown' root
	m_UnknownItemsRoot = IsUnknownEntriesVisible() ? CreateUnknownItemsRoot() : m_TreeRoot;
	m_CategoriesTable.emplace(std::make_pair(wxEmptyString, m_UnknownItemsRoot));
}
void KCMController::Reload()
{
	GetConfigManager()->Reload();
	LoadView();
}
void KCMController::LoadView()
{
	wxWindowUpdateLocker redrawLock(m_View);
	ResetView();

	KxTreeListItem tENBRoot = (IsENBAllowed() && m_ConfigManager->GetGameConfig()->IsENBEnabled()) ? GetCategoryNode("ENB") : KxTreeListItem();
	for (size_t nFileIndex = 0; nFileIndex < m_ConfigManager->GetEntriesCount(); nFileIndex++)
	{
		KCMFileEntry* fileEntry = m_ConfigManager->GetEntryAt(nFileIndex);
		if (fileEntry && fileEntry->IsOK())
		{
			// ENB items have to be in its own branch
			KxTreeListItem tExtraRoot;
			wxString sCategoryPath;
			if (fileEntry->IsPartOfENB())
			{
				sCategoryPath = "ENB/";
				tExtraRoot = tENBRoot;
			}

			for (size_t nEntryIndex = 0; nEntryIndex < fileEntry->GetEntriesCount(); nEntryIndex++)
			{
				KCMConfigEntryBase* entry = fileEntry->GetEntryAt(nEntryIndex);

				if (!IsUnknownEntriesVisible() && entry->IsUnknownEntry())
				{
					continue;
				}

				if (KCMConfigEntryStd* configEntry = entry->ToStdEntry())
				{
					// Add to hash and proceed only if this is new item
					if (AddToHash(configEntry))
					{
						// All unknown items (including unknown ENB items) must be in 'unknown' branch
						if (configEntry->IsUnknownEntry())
						{
							sCategoryPath.Clear();
							tExtraRoot = m_UnknownItemsRoot;
						}

						// Only known items can be localized
						bool bLocalize = !configEntry->IsUnknownEntry();

						KxTreeListItem tRoot = GetCategoryNode(sCategoryPath + configEntry->GetCategory(), tExtraRoot, bLocalize);
						KxTreeListItem item = tRoot.Add(OnFormatEntryToView(configEntry));

						configEntry->SetViewNode(item);
						item.SetData(new KCMConfigEntryRef(configEntry));
					}
				}
				else if (KCMConfigEntryPath* pPath = entry->ToPathEntry())
				{
					if (pPath->IsUnknownEntry())
					{
						KxTreeListItem item = GetCategoryNode(pPath->GetFullPath(), m_UnknownItemsRoot, false);
						pPath->SetViewNode(item);
						item.SetData(new KCMConfigEntryRef(pPath));
					}
				}
			}
		}
	}

	if (ShouldExpandTopLevelNodes())
	{
		for (KxTreeListItem node = m_TreeRoot.GetFirstChild(); node.IsOK(); node = node.GetNextSibling())
		{
			node.Expand();
		}
	}
}
wxString KCMController::GetStatusBarString(KCMConfigEntryBase* entry) const
{
	wxString sType;
	wxString sData;
	if (KCMConfigEntryDV* pDV = entry->ToDVEntry())
	{
		sType = "struct";
		sData = wxString::Format("{%s %s = %s, %s %s = %s}", pDV->GetTypeName(), pDV->GetName1(), pDV->FormatToDisplay(pDV->GetData1()), pDV->GetTypeName(), pDV->GetName2(), pDV->FormatToDisplay(pDV->GetData2()));
	}
	else if (KCMConfigEntryStd* stdEntry = entry->ToStdEntry())
	{
		sType = stdEntry->GetTypeName();
		if (stdEntry->FindDataInSamples() != wxNOT_FOUND)
		{
			sData = stdEntry->GetData();
		}
		else
		{
			sData = stdEntry->GetDisplayData();
		}
	}

	if (KCMConfigEntryStd* stdEntry = entry->ToStdEntry())
	{
		return wxString::Format("%s %s = %s", sType, stdEntry->GetName(), sData);
	}
	return wxEmptyString;
}

bool KCMController::HasUnsavedChanges() const
{
	return !m_ModifiedEntries.empty();
}
void KCMController::SaveChanges()
{
	if (HasUnsavedChanges())
	{
		std::unordered_set<KCMIDataProvider*> tDataProviders;

		for (auto& v: m_ModifiedEntries)
		{
			KCMFileEntry* fileEntry = v.first->GetFileEntry();
			if (fileEntry && fileEntry->IsOK())
			{
				KCMIDataProvider* pDataProvider = fileEntry->GetProvider();
				tDataProviders.emplace(pDataProvider);

				switch (v.second)
				{
					case KCMCC_REMOVE_PATH:
					{
						if (KCMConfigEntryPath* pStd = v.first->ToPathEntry())
						{
							pDataProvider->ProcessRemovePath(pStd);
						}
						break;
					}
					case KCMCC_REMOVE_ENTRY:
					{
						if (KCMConfigEntryStd* pStd = v.first->ToStdEntry())
						{
							pDataProvider->ProcessRemoveEntry(pStd);
						}
						break;
					}
					case KCMCC_MODIFY_ENTRY:
					{
						if (KCMConfigEntryStd* pStd = v.first->ToStdEntry())
						{
							pDataProvider->ProcessSaveEntry(pStd);
						}
						break;
					}
					case KCMCC_CREATE_ENTRY:
					{
						break;
					}
				};
			}
		}

		// Save changed files
		for (auto& pDataProvider: tDataProviders)
		{
			pDataProvider->Save();
		}

		// All saved, now clear items
		DiscardChangesHelper();
		Reload();

		wxNotifyEvent event(KEVT_CONTROLLER_SAVED);
		ProcessEvent(event);
	}
}
void KCMController::DiscardChanges()
{
	if (HasUnsavedChanges())
	{
		DiscardChangesHelper();
		Reload();

		wxNotifyEvent event(KEVT_CONTROLLER_DISCARDED);
		ProcessEvent(event);
	}
}

//////////////////////////////////////////////////////////////////////////
bool KCMCAddEntryDialog::Create(wxWindow * 	parent,
								wxWindowID 	id,
								const wxString& caption,
								const wxPoint & 	pos,
								const wxSize & 	size,
								int buttons,
								long 	style
)
{
	if (KxComboBoxDialog::Create(parent, id, caption, pos, size, buttons, style))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxHORIZONTAL);
		wxWindow* canvas = GetContentWindow();
		wxSizer* mainSizer = GetContentWindowSizer();

		CreateFilesList();
		CreatePathInput();
		CreateNameInput();
		
		AdjustWindow(pos);
		Bind(KxEVT_STDDIALOG_BUTTON, &KCMCAddEntryDialog::OnOK, this);
		return true;
	}
	return false;
}
void KCMCAddEntryDialog::CreateFilesList()
{
	m_FilesList = static_cast<KxComboBox*>(GetDialogMainCtrl());
	m_FilesList->Enable(false);

	int sel = 0;
	for (size_t i = 0; i < m_Controller->GetConfigManager()->GetEntriesCount(); i++)
	{
		KCMFileEntry* fileEntry = m_Controller->GetConfigManager()->GetEntryAt(i);
		if (fileEntry && fileEntry->IsOK())
		{
			int index = m_FilesList->AddItem(fileEntry->GetProfileEntry()->GetFileName());
			if (fileEntry == m_ConfigEntry->GetFileEntry())
			{
				sel = index;
			}
		}
	}
	m_FilesList->SetSelection(sel);
}
void KCMCAddEntryDialog::CreatePathInput()
{
	m_PathInput = new KxTextBox(GetContentWindow(), KxID_NONE);
	m_PathInput->SetHint(T("ConfigManager.EditValue.CreateValue.PathHint"));
	if (KCMConfigEntryPath* pPath = m_ConfigEntry->ToPathEntry())
	{
		m_PathInput->SetValue(pPath->GetPath());
	}
	GetContentWindowSizer()->Add(m_PathInput, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
	AddUserWindow(m_PathInput);
}
void KCMCAddEntryDialog::CreateNameInput()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

	// Type
	m_TypeList = new KxComboBox(GetContentWindow(), KxID_NONE);
	for (int i = 0; i < KCMDT_MAX; i++)
	{
		m_TypeList->AddItem(KConfigManager::GetTypeName((KCMDataType)i));
	}
	sizer->Add(m_TypeList, 0);
	AddUserWindow(m_TypeList);

	// Name
	m_NameInput = new KxTextBox(GetContentWindow(), KxID_NONE);
	m_NameInput->SetHint(T("ConfigManager.EditValue.CreateValue.ValueNameHint"));
	if (KCMConfigEntryStd* pStd = m_ConfigEntry->ToStdEntry())
	{
		m_NameInput->SetValue(pStd->GetName());
		m_TypeList->SetValue(pStd->GetTypeName());
	}
	else
	{
		m_TypeList->SetSelection(0);
	}
	sizer->Add(m_NameInput, 1, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING);
	AddUserWindow(m_NameInput);

	GetContentWindowSizer()->Add(sizer, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
}

void KCMCAddEntryDialog::OnOK(wxNotifyEvent& event)
{
	if (event.GetId() == KxID_OK)
	{
		KCMConfigEntryStd* newEntry = new KCMConfigEntryStd(m_ConfigEntry->GetFileEntry(), m_ConfigEntry->GetFileEntry()->GetFormatter());
		if (!m_Controller->IsInHash(newEntry))
		{
			newEntry->Create(m_PathInput->GetValue(), m_NameInput->GetValue(), (KCMDataType)m_TypeList->GetSelection());

			KxTreeListItem tRoot = m_Controller->GetCategoryNode(newEntry->GetFullPathFor(wxEmptyString));
			KxTreeListItem item = tRoot.Add(m_Controller->OnFormatEntryToView(newEntry));
			item.EnsureVisible();

			newEntry->SetViewNode(item);
			item.SetData(new KCMConfigEntryRef(newEntry));
			m_Controller->OnAddModifiedEntry(newEntry, KCMCC_CREATE_ENTRY);
		}
		else
		{
			delete newEntry;
			KLogEvent(T("ConfigManager.EditValue.CreateValue.NameCollision"), KLOG_WARNING, this).Send();
			event.Veto();
		}
	}
	else
	{
		event.Skip();
	}
}

KCMCAddEntryDialog::KCMCAddEntryDialog(KCMController* controller, KCMConfigEntryBase* configEntry)
	:m_Controller(controller), m_ConfigEntry(configEntry)
{
	Create(m_Controller->GetParentWindow(), KxID_NONE, T("ConfigManager.EditValue.CreateValue.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY);
}
KCMCAddEntryDialog::~KCMCAddEntryDialog()
{
}
