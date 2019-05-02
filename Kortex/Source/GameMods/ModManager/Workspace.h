#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
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
class KVFSEvent;
class KxSearchBox;
class KxHTMLWindow;
class KxParagraph;

namespace Kortex
{
	class IModTag;
	class IGameMod;
}

namespace Kortex::ModManager
{
	class DisplayModel;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		public:
			enum class ImageResizeMode
			{
				Scale,
				Stretch,
				Fill,
			};

		private:
			ImageResizeMode m_ImageResizeMode = ImageResizeMode::Scale;

			wxBoxSizer* m_MainSizer = nullptr;
			KxSplitterWindow* m_SplitterLeftRight = nullptr;

			/* Mods pane */
			wxBoxSizer* m_ModsPaneSizer = nullptr;

			/* ToolBar */
			KxAuiToolBar* m_ModsToolBar = nullptr;

			KxComboBox* m_ToolBar_Profiles = nullptr;
			KxAuiToolBarItem* m_ToolBar_EditProfiles = nullptr;

			KxAuiToolBarItem* m_ToolBar_AddMod = nullptr;
			KxMenu* m_ToolBar_AddModMenu = nullptr;

			KxAuiToolBarItem* m_ToolBar_ChangeDisplayMode = nullptr;
			KxMenu* m_ToolBar_DisplayModeMenu = nullptr;

			KxAuiToolBarItem* m_ToolBar_Tools = nullptr;

			// Mod manager
			KxPanel* m_ModsPane = nullptr;
			DisplayModel* m_DisplayModel = nullptr;

			/* Right pane */
			KxAuiNotebook* m_PaneRight_Tabs = nullptr;

			/* Controls */
			KxButton* m_ActivateButton = nullptr;
			KxSearchBox* m_SearchBox = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
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
			virtual ResourceID GetImageID() const override
			{
				return ImageResourceID::Puzzle;
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
			bool ShowChangeModIDDialog(IGameMod* entry);

			void ProcessSelectProfile(const wxString& newProfileID);
			void OnSelectProfile(wxCommandEvent& event);
			void OnShowProfileEditor(KxAuiToolBarEvent& event);
		
			void OnDisplayModeMenu(KxAuiToolBarEvent& event);
			void OnToolsMenu(KxAuiToolBarEvent& event);
			void OnVFSToggled(VFSEvent& event);
		
			void OnAddMod_Empty(KxMenuEvent& event);
			void OnAddMod_FromFolder(KxMenuEvent& event);
			void OnAddMod_InstallPackage(KxMenuEvent& event);

			void InstallMod(IGameMod* entry);
			void UninstallMod(IGameMod* entry, bool eraseLog);
			void OnModSerach(wxCommandEvent& event);
			void OnModSearchColumnsChanged(KxMenuEvent& event);

			void ClearControls();
			void DisplayModInfo(IGameMod* entry);
			void CreateViewContextMenu(KxMenu& contextMenu, const IGameMod::RefVector& selectedMods, IGameMod* focusedMod);
			void ChangeImageResizeMode(ImageResizeMode mode);

		public:
			DisplayModel* GetModel() const
			{
				return m_DisplayModel;
			}
			ImageResizeMode GetImageResizeMode() const
			{
				return m_ImageResizeMode;
			}

			void OpenPackage(const wxString& path);
			void SelectMod(const IGameMod* entry);
			void ProcessSelection(IGameMod* entry = nullptr);
			void HighlightMod(const IGameMod* entry = nullptr);
			void ReloadView();

			void OnModsContextMenu(const IGameMod::RefVector& selectedMods, IGameMod* focusedMod);
			void UpdateModListContent();

			bool IsAnyChangeAllowed() const;
			bool IsMovingModsAllowed() const;
			bool IsChangingModsAllowed() const;
	};
}
