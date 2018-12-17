#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxSingleton.h>
class KImageViewerEvent;
class KxTextBox;
class KxThumbView;

namespace Kortex
{
	class IScreenshotsGallery;
}

namespace Kortex::ScreenshotsGallery
{
	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			IScreenshotsGallery* m_Manager = nullptr;

			wxBoxSizer* m_MainSizer = nullptr;
			KxThumbView* m_ViewPane = nullptr;
			KxStringVector m_LoadedImages;
			int m_CurrentImageIndex = -1;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			void LoadData();
			void OnSelectItem(wxCommandEvent& event);
			void OnActivateItem(wxCommandEvent& event);
			void OnItemMenu(wxContextMenuEvent& event);
			void OnDialogNavigate(KImageViewerEvent& event);
			void SetNavigationInfo(KImageViewerEvent& event);

			virtual bool OnOpenWorkspace() override;
			virtual bool OnCloseWorkspace() override;
			virtual void OnReloadWorkspace() override;

		public:
			virtual wxString GetID() const override;
			virtual wxString GetName() const override;
			virtual KImageEnum GetImageID() const override
			{
				return KIMG_PICTURES;
			}
			virtual wxSizer* GetWorkspaceSizer() const override
			{
				return m_MainSizer;
			}
			virtual bool CanReload() const override
			{
				return true;
			}

		private:
			void DisplayInfo(const wxString& filePath);
			void ClearControls();
	};
}
