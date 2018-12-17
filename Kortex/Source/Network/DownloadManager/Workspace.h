#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex::DownloadManager
{
	class DisplayModel;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel* m_ViewModel = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			virtual bool IsSubWorkspace() const override
			{
				return true;
			}
			virtual size_t GetTabIndex() const
			{
				return (size_t)TabIndex::Downloads;
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
			DisplayModel* GetModelView() const
			{
				return m_ViewModel;
			}
	};
}
