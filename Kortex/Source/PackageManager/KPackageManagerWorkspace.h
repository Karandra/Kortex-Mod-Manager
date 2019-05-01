#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "Application/Resources/ImageResourceID.h"
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
		//KProgramOptionAI m_PackageListViewOptions;
		//KProgramOptionAI m_MainOptions;
		
		/* Layout */
		wxBoxSizer* m_MainSizer = nullptr;
		wxBoxSizer* m_ViewSizer = nullptr;
		KxSplitterWindow* m_Splitter = nullptr;

		/* View */
		wxWindow* m_ViewPane = nullptr;
		KxAuiToolBar* m_ToolBar = nullptr;
		KxSearchBox* m_SearchBox = nullptr;
		KPackageManagerListModel* m_MainView = nullptr;

		/* Info pane */
		KxSplitterWindow* m_InfoPane = nullptr;
		KxImageView* m_InfoImage = nullptr;
		KxHTMLWindow* m_InfoDescription = nullptr;

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
		virtual Kortex::ResourceID GetImageID() const override
		{
			return Kortex::ImageResourceID::CategoryItem;
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
