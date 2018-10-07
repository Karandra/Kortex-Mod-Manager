#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KDownloadManager;
class KDownloadView;

class KDownloadWorkspace: public KWorkspace, public KxSingletonPtr<KDownloadWorkspace>
{
	private:
		wxBoxSizer* m_MainSizer = NULL;
		KDownloadView* m_ViewModel = NULL;

		KProgramOptionUI m_ViewOptions;

	public:
		KDownloadWorkspace(KMainWindow* mainWindow, KDownloadManager* manager);
		virtual ~KDownloadWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual bool IsSubWorkspace() const override
		{
			return true;
		}
		virtual size_t GetTabIndex() const
		{
			return KWS_TABINDEX_DOWNLOADS;
		}
		
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetNameShort() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_ARROW_270;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		virtual bool CanReload() const override
		{
			return true;
		}

	public:
		KDownloadView* GetModelView() const
		{
			return m_ViewModel;
		}
};