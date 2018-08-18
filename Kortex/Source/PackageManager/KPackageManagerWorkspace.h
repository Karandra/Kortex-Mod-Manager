#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxSingleton.h>
class KxHTMLWindow;
class KxSearchBox;
class KPackageManagerListModel;

class KPackageManagerWorkspace: public KWorkspace, public KxSingletonPtr<KPackageManagerWorkspace>
{
	private:
		KProgramOptionUI m_PackageListViewOptions;
		KProgramOptionUI m_MainOptions;

		/* Layout */
		wxBoxSizer* m_MainSizer = NULL;
		wxBoxSizer* m_ViewSizer = NULL;
		KxSplitterWindow* m_Splitter = NULL;

		/* View */
		wxWindow* m_ViewPane = NULL;
		KxAuiToolBar* m_ToolBar = NULL;
		KxSearchBox* m_SearchBox = NULL;
		KPackageManagerListModel* m_MainView = NULL;

		/* Info pane */
		KxSplitterWindow* m_InfoPane = NULL;
		KxImageView* m_InfoImage = NULL;
		KxHTMLWindow* m_InfoDescription = NULL;

	public:
		KPackageManagerWorkspace(KMainWindow* mainWindow);
		virtual ~KPackageManagerWorkspace();
		virtual bool OnCreateWorkspace() override;

		void CreateViewPane();
		void CreateInfoPane();

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

		void OnSerach(wxCommandEvent& event);
		
	protected:
		virtual bool DoCanBeStartPage() const override
		{
			return true;
		}

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_CATEGORY_ITEM;
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
