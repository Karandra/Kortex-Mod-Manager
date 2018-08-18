#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
class KGameConfigWorkspaceController;

class KGameConfigWorkspace: public KWorkspace, public KxSingletonPtr<KGameConfigWorkspace>
{
	private:
		wxBoxSizer* m_MainSizer = NULL;
		wxBoxSizer* m_ListViewSizer = NULL;
		wxBoxSizer* m_ControlsSizer = NULL;

		/* Config list */
		std::unordered_map<wxString, KxTreeListItem> m_CategoriesTable;

		/* Controls */
		KxButton* m_SaveButton = NULL;
		KxButton* m_DiscardButton = NULL;

		/* ConfigManager */
		KProgramOptionUI m_GameConfigViewOptions;
		KGameConfigWorkspaceController* m_Controller;
		KxTreeList* m_ControllerView = NULL;

	public:
		KGameConfigWorkspace(KMainWindow* mainWindow);

	private:
		void CreateControllerView();
		virtual bool OnCreateWorkspace() override;
		virtual ~KGameConfigWorkspace();

		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;
		virtual bool DoCanBeStartPage() const
		{
			return true;
		}

		void OnSaveButton(wxCommandEvent& event);
		void OnDiscardButton(wxCommandEvent& event);
		void OnControllerSaveDiscard(wxNotifyEvent& event);

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_GEAR;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		virtual bool CanReload() const override
		{
			return true;
		}
};
