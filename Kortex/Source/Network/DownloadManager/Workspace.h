#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>

namespace Kortex::DownloadManager
{
	class DisplayModel;

	class Workspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<Workspace>
	{
		private:
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel* m_DisplayModel = nullptr;
			KxAuiToolBar* m_ToolBar = nullptr;

		private:
			void OnSelectConcurrentDownloadsCount(wxCommandEvent& event);

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			Workspace() = default;
			~Workspace();

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const override;
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Arrow270;
			}
			IWorkspaceContainer* GetPreferredContainer() const override;

		public:
			DisplayModel* GetDisplayModel() const
			{
				return m_DisplayModel;
			}
	};
}
