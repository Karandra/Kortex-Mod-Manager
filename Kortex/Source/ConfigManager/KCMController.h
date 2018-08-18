#pragma once
#include "stdafx.h"
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxComboBoxDialog.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxMenu.h>
#include "KConfigManager.h"
#include "UI/KWorkspaceController.h"
class KWorkspace;
class KxStdDialog;
class KxComboBoxDialog;
class KxTextBoxDialog;
class KxListBoxDialog;
class KxFileBrowseDialog;

enum KCMControllerCommands
{
	KCMCC_CREATE_ENTRY,
	KCMCC_MODIFY_ENTRY,
	KCMCC_REMOVE_ENTRY,
	KCMCC_REMOVE_PATH,
};

class KCMCAddEntryDialog;
class KCMController: public KWorkspaceController
{
	friend class KCMCAddEntryDialog;

	private:
		std::unordered_map<wxString, KxTreeListItem> m_CategoriesTable;
		std::unordered_set<wxString> m_AddedItems;
		KxTreeListItem m_TreeRoot;
		KxTreeListItem m_UnknownItemsRoot;

		KxMenu* m_ContextMenu = NULL;
		KxMenuItem* m_ContextMenu_EditValue = NULL;
		KxMenuItem* m_ContextMenu_RemoveValue = NULL;
		KxMenuItem* m_ContextMenu_RemovePath = NULL;
		KxMenuItem* m_ContextMenu_CreateValue = NULL;

	protected:
		KxTreeList* m_View = NULL;
		KConfigManager* m_ConfigManager = NULL;
		std::vector<std::pair<KCMConfigEntryBase*, KCMControllerCommands>> m_ModifiedEntries;

	private:
		void CreateContextMenu();

		KxTreeListItem GetCategoryNode(const wxString& sCategoryPath = wxEmptyString, const KxTreeListItem& tExtraRoot = KxTreeListItem(), bool bLocalize = false);
		void AddSampleValues(KxComboBoxDialog* dialog, KCMConfigEntryStd* configEntry);
		KxDialog* CreateDialog(KCMConfigEntryStd* configEntry);
		wxString GetDialogValue(KxDialog* dialog, KCMConfigEntryStd* configEntry);
		void SetDialogValidator(KxDialog* dialog, KCMConfigEntryStd* configEntry);
		void ConfigureForVirtualKey(KxStdDialog* dialog, KCMConfigEntryVK* pVKEntry);
		bool AddToHash(KCMConfigEntryPath* configEntry);
		bool IsInHash(KCMConfigEntryPath* configEntry);
		void UpdateItemImages(KCMConfigEntryBase* entry, KImageEnum image, bool recursive = false);
		void DiscardChangesHelper();

	protected:
		const wxString& GetCategoryName(const wxString& sCategoryID) const;
		KCMConfigEntryRef* GetEntryFromItem(const KxTreeListItem& item) const;

		virtual KxComboBoxDialog* OnCreateComboBoxDialog(KCMConfigEntryStd* configEntry);
		virtual KxTextBoxDialog* OnCreateTextBoxDialog(KCMConfigEntryStd* configEntry);
		virtual KxListBoxDialog* OnCreateListBoxDialog(KCMConfigEntryArray* configEntry);
		virtual KxFileBrowseDialog* OnCreateFileBrowseDialog(KCMConfigEntryFileBrowse* configEntry);
		virtual void OnConfigureDialog(KxDialog* dialog, KCMConfigEntryStd* configEntry);
		virtual wxWindow* GetParentWindow();

		virtual int GetValueNameColumn() const
		{
			return 1;
		}
		virtual int GetValueDataColumn() const
		{
			return 3;
		}
		virtual KxTreeListItem CreateUnknownItemsRoot();
		virtual KxStringVector OnFormatEntryToView(KCMConfigEntryPath* pathEntry);
		virtual KxStringVector OnFormatEntryToView(KCMConfigEntryStd* stdEntry);
		virtual void OnEditEntry(KCMConfigEntryStd* configEntry);
		virtual void OnCreateEntry(KCMConfigEntryBase* entry);
		virtual void OnRemoveEntry(KCMConfigEntryStd* configEntry, KCMConfigEntryRef* entryRef);
		virtual void OnRemovePath(KCMConfigEntryPath* configEntry, KCMConfigEntryRef* entryRef);
		virtual void OnAddModifiedEntry(KCMConfigEntryBase* entry, KCMControllerCommands nCommand);
		void OnActivateView(wxTreeListEvent& event);
		void OnSelectView(wxTreeListEvent& event);
		void OnContextMenu(wxTreeListEvent& event);

	public:
		KCMController(KWorkspace* workspace, KConfigManager* pConfigManager, KxTreeList* view);
		virtual ~KCMController();

	public:
		virtual bool IsOK() const override;

		virtual bool ShouldExpandTopLevelNodes() const
		{
			return false;
		}
		virtual bool IsUnknownEntriesVisible() const
		{
			return true;
		}
		virtual bool IsENBAllowed() const
		{
			return true;
		}
		virtual bool IsEditable() const
		{
			return true;
		}
		virtual bool IsCreateEntryAllowed() const
		{
			return true;
		}
		virtual bool IsRemoveEntryAllowed() const
		{
			return true;
		}
		virtual bool IsRemovePathAllowed() const
		{
			return true;
		}

		KxTreeListItem GetTreeRoot() const
		{
			return m_TreeRoot;
		}
		KxTreeListItem GetUnknownItemsRoot() const
		{
			return m_UnknownItemsRoot;
		}
		KConfigManager* GetConfigManager() const
		{
			return m_ConfigManager;
		}
		KxTreeList* GetView() const
		{
			return m_View;
		}
		virtual wxString GetStatusBarString(KCMConfigEntryBase* entry) const;

	protected:
		virtual void ResetView() override;

	public:
		virtual void Reload() override;
		virtual void LoadView() override;

		bool HasUnsavedChanges() const override;
		void SaveChanges() override;
		void DiscardChanges() override;
};

//////////////////////////////////////////////////////////////////////////
class KCMCAddEntryDialog: public KxComboBoxDialog
{
	private:
		KCMController* m_Controller = NULL;
		KCMConfigEntryBase* m_ConfigEntry = NULL;

		KxComboBox* m_FilesList = NULL;
		KxTextBox* m_PathInput = NULL;
		KxTextBox* m_NameInput = NULL;
		KxComboBox* m_TypeList = NULL;

	private:
		bool Create(wxWindow * 	parent,
					wxWindowID 	id,
					const wxString& caption,
					const wxPoint & 	pos = wxDefaultPosition,
					const wxSize & 	size = wxDefaultSize,
					int buttons = DefaultButtons,
					long 	style = DefaultStyle
		);
		void CreateFilesList();
		void CreatePathInput();
		void CreateNameInput();

	private:
		void OnOK(wxNotifyEvent& event);
	
	public:
		KCMCAddEntryDialog(KCMController* controller, KCMConfigEntryBase* configEntry);
		virtual ~KCMCAddEntryDialog();
};
