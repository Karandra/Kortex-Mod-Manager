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
			DisplayModel* m_DisplayModel = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			~Workspace();
			bool OnCreateWorkspace() override;

		private:
			bool IsSubWorkspace() const override
			{
				return true;
			}
			size_t GetTabIndex() const
			{
				return (size_t)TabIndex::Downloads;
			}
		
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const override;
			ResourceID GetImageID() const override
			{
				return ImageResourceID::Arrow270;
			}
			wxSizer* GetWorkspaceSizer() const override
			{
				return m_MainSizer;
			}
			bool CanReload() const override
			{
				return true;
			}

		public:
			DisplayModel* GetDisplayModel() const
			{
				return m_DisplayModel;
			}
	};
}
