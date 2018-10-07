#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KProgramOptions.h"
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxSingleton.h>
class KModTag;
class KModEntry;
class KModManagerModel;
class KModManagerLogModel;
class KVFSEvent;

class KxSearchBox;
class KxHTMLWindow;
class KxParagraph;

class KModWorkspace: public KWorkspace, public KxSingletonPtr<KModWorkspace>
{
	public:
		enum class ImageResizeMode
		{
			Scale,
			Stretch,
			Fill,
		};

	private:
		KProgramOptionUI m_OptionsUI;
		KProgramOptionUI m_ModListViewOptions;
		ImageResizeMode m_ImageResizeMode = ImageResizeMode::Scale;

		wxBoxSizer* m_MainSizer = NULL;
		KxSplitterWindow* m_SplitterLeftRight = NULL;

		/* Mods pane */
		wxBoxSizer* m_ModsPaneSizer = NULL;

		/* ToolBar */
		KxAuiToolBar* m_ModsToolBar = NULL;

		KxComboBox* m_ToolBar_ModList = NULL;
		KxAuiToolBarItem* m_ToolBar_ManageModList = NULL;

		KxAuiToolBarItem* m_ToolBar_AddMod = NULL;
		KxMenu* m_ToolBar_AddModMenu = NULL;

		KxAuiToolBarItem* m_ToolBar_ChangeDisplayMode = NULL;
		KxMenu* m_ToolBar_DisplayModeMenu = NULL;

		KxAuiToolBarItem* m_ToolBar_Tools = NULL;

		// Mod manager
		KxPanel* m_ModsPane = NULL;
		KModManagerModel* m_ViewModel = NULL;

		/* Right pane */
		KxAuiNotebook* m_PaneRight_Tabs = NULL;

		/* Controls */
		KxButton* m_ActivateButton = NULL;
		KxSearchBox* m_SearchBox = NULL;

	public:
		KModWorkspace(KMainWindow* mainWindow);
		virtual ~KModWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		void CreateToolBar();
		void CreateDisplayModeMenu();
		void CreateAddModMenu();
		void CreateToolsMenu();

		void CreateModsView();
		void CreateControls();
		void CreateRightPane();

		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;
		virtual bool DoCanBeStartPage() const
		{
			return true;
		}

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_PUZZLE;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		virtual bool CanReload() const override
		{
			return true;
		}

		virtual bool AddSubWorkspace(KWorkspace* workspace) override;
		virtual wxBookCtrlBase* GetSubWorkspaceContainer() override
		{
			return m_PaneRight_Tabs;
		}
		void OnSubWorkspaceOpening(wxAuiNotebookEvent& event);
		void OnSubWorkspaceOpened(wxAuiNotebookEvent& event);

	private:
		void OnMountButton(wxCommandEvent& event);
		bool ShowChangeModIDDialog(KModEntry* entry);
		
		void OnDisplayModeMenu(KxAuiToolBarEvent& event);
		void OnToolsMenu(KxAuiToolBarEvent& event);
		void OnVFSToggled(KVFSEvent& event);
		
		void OnAddMod_Empty(KxMenuEvent& event);
		void OnAddMod_FromFolder(KxMenuEvent& event);
		void OnAddMod_InstallPackage(KxMenuEvent& event);

		void InstallMod(KModEntry* entry);
		void UninstallMod(KModEntry* entry, bool eraseLog);
		void OnModSerach(wxCommandEvent& event);
		void OnModSearchColumnsChanged(KxMenuEvent& event);

		void ClearControls();
		void DisplayModInfo(KModEntry* entry);
		void CreateViewContextMenu(KxMenu& contextMenu, KModEntry* entry);
		void ChangeImageResizeMode(ImageResizeMode mode);

	public:
		KModManagerModel* GetModel() const
		{
			return m_ViewModel;
		}
		ImageResizeMode GetImageResizeMode() const
		{
			return m_ImageResizeMode;
		}

		void OpenPackage(const wxString& path);
		void SelectMod(const KModEntry* entry);
		void ProcessSelection(KModEntry* entry = NULL);
		void HighlightMod(const KModEntry* entry = NULL);
		void ReloadView();

		void ShowViewContextMenu(KModEntry* entry);
		void ShowViewContextMenu(const KModTag* modTag);
		void UpdateModListContent();
		void RefreshPlugins();

		bool IsAnyChangeAllowed() const;
		bool IsMovingModsAllowed() const;
		bool IsChangingModsAllowed() const;
};
