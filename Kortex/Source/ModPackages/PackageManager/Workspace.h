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
class KxHTMLWindow;
class KxSearchBox;

namespace Kortex::PackageManager
{
	class DisplayModel;
}

namespace Kortex::PackageManager
{
	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			// Layout
			wxBoxSizer* m_MainSizer = nullptr;
			wxBoxSizer* m_ViewSizer = nullptr;
			KxSplitterWindow* m_Splitter = nullptr;

			// View
			wxWindow* m_ViewPane = nullptr;
			KxAuiToolBar* m_ToolBar = nullptr;
			KxSearchBox* m_SearchBox = nullptr;
			DisplayModel* m_DisplayModel = nullptr;

			// Info pane
			KxSplitterWindow* m_InfoPane = nullptr;
			KxImageView* m_InfoImage = nullptr;
			KxHTMLWindow* m_InfoDescription = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			~Workspace();
			bool OnCreateWorkspace() override;

			void CreateViewPane();
			void CreateInfoPane();

		private:
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

			void OnSerach(wxCommandEvent& event);
		
		protected:
			bool DoCanBeStartPage() const override
			{
				return true;
			}

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			ResourceID GetImageID() const override
			{
				return Kortex::ImageResourceID::CategoryItem;
			}
			wxSizer* GetWorkspaceSizer() const override
			{
				return m_MainSizer;
			}

			bool CanReload() const override
			{
				return true;
			}
	};
}
