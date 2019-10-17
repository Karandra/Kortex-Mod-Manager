#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include "Application/BookWorkspaceContainer.h"
#include "GameMods/IGameMod.h"
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxSingleton.h>
class KxSearchBox;
class KxMenuEvent;

namespace Kortex
{
	class IModTag;
	class ModEvent;
	class ProfileEvent;
	class VirtualFSEvent;
}

namespace Kortex::ModManager
{
	class Workspace;
	class WorkspaceContainer: public Application::BookWorkspaceContainer
	{
		friend class Workspace;

		private:
			Workspace& m_Workspace;
			KxAuiNotebook* m_BookCtrl = nullptr;

		private:
			void Create(wxWindow* parent);
			void OnPageOpening(wxAuiNotebookEvent& event);
			void OnPageOpened(wxAuiNotebookEvent& event);

		public:
			WorkspaceContainer(Workspace& workspace)
				:m_Workspace(workspace)
			{
			}

		public:
			wxWindow& GetWindow() override
			{
				return *m_BookCtrl;
			}
			IWorkspaceContainer* GetParentContainer() override;
	};
}

namespace Kortex::ModManager
{
	class DisplayModel;
	class Workspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<Workspace>
	{
		friend class WorkspaceContainer;

		private:
			BroadcastReciever m_BroadcastReciever;

			wxBoxSizer* m_MainSizer = nullptr;
			KxSplitterWindow* m_SplitterLeftRight = nullptr;

			// Left pane
			wxBoxSizer* m_LeftPaneSizer = nullptr;

			// ToolBar
			KxAuiToolBar* m_ModsToolBar = nullptr;

			KxComboBox* m_ToolBar_Profiles = nullptr;
			KxAuiToolBarItem* m_ToolBar_EditProfiles = nullptr;

			KxAuiToolBarItem* m_ToolBar_AddMod = nullptr;
			KxMenu* m_ToolBar_AddModMenu = nullptr;

			KxAuiToolBarItem* m_ToolBar_ChangeDisplayMode = nullptr;
			KxMenu* m_ToolBar_DisplayModeMenu = nullptr;

			KxAuiToolBarItem* m_ToolBar_Tools = nullptr;
			KxSearchBox* m_SearchBox = nullptr;

			// Mod manager
			KxPanel* m_LeftPaneWindow = nullptr;
			DisplayModel* m_DisplayModel = nullptr;

			// Right pane
			KxPanel* m_RightPaneWindow = nullptr;
			WorkspaceContainer m_RightPaneContainer;
			KxButton* m_ActivateFSButton = nullptr;

		private:
			void CreateLeftPane();
			void CreateLeftPaneModList();
			void CreateLeftPaneToolbar();
			void CreateLeftPaneToolbar_DisplayMode();
			void CreateLeftPaneToolbar_AddMod();
			void CreateLeftPaneToolbar_Tools();
			void CreateRightPane();

		private:
			void OnMountButton(wxCommandEvent& event);
			bool ShowChangeModIDDialog(IGameMod& mod);

			void ProcessSelectProfile(const wxString& newProfileID);
			void OnSelectProfile(wxCommandEvent& event);
			void OnShowProfileEditor(KxAuiToolBarEvent& event);
			void OnUpdateModLayoutNeeded(ModEvent& event);
		
			void OnDisplayModeMenu(KxAuiToolBarEvent& event);
			void OnToolsMenu(KxAuiToolBarEvent& event);
			void OnMainFSToggled(VirtualFSEvent& event);
			void OnProfileSelected(ProfileEvent& event);
		
			void OnAddMod_Empty(KxMenuEvent& event);
			void OnAddMod_FromFolder(KxMenuEvent& event);
			void OnAddMod_InstallPackage(KxMenuEvent& event);

			void InstallMod(IGameMod& mod);
			void UninstallMod(IGameMod& mod, bool eraseLog);
			void OnModSerach(wxCommandEvent& event);
			void OnModSearchColumnsChanged(KxMenuEvent& event);

			void ClearControls();
			void DisplayModInfo(IGameMod* entry);
			void CreateViewContextMenu(KxMenu& contextMenu, const IGameMod::RefVector& selectedMods, IGameMod* focusedMod);

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			Workspace()
				:m_RightPaneContainer(*this)
			{
			}
			~Workspace();

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Puzzle;
			}
			
			IWorkspaceContainer* GetPreferredContainer() const override;
			IWorkspaceContainer& GetWorkspaceContainer()
			{
				return m_RightPaneContainer;
			}

		public:
			DisplayModel* GetModel() const
			{
				return m_DisplayModel;
			}

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
