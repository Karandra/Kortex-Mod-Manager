#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
class KScreenshotsGalleryManager;
class KImageViewerEvent;
class KxTextBox;
class KxThumbView;

class KScreenshotsGalleryWorkspace: public KWorkspace
{
	private:
		KScreenshotsGalleryManager* m_Manager = NULL;

		/* Layout */
		wxBoxSizer* m_MainSizer = NULL;

		/* View */
		KxThumbView* m_ViewPane = NULL;
		KxStringVector m_LoadedImages;
		int m_CurrentImageIndex = -1;

	public:
		KScreenshotsGalleryWorkspace(KMainWindow* mainWindow, KScreenshotsGalleryManager* manager);
		virtual ~KScreenshotsGalleryWorkspace();
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
