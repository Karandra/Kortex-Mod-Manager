#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class VirtualFSEvent;
}

namespace Kortex::ProgramManager
{
	class DisplayModel;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			BroadcastReciever m_BroadcastReciever;
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel* m_ViewModel = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

			void OnMainFSToggled(VirtualFSEvent& event);

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const override;

			ResourceID GetImageID() const override
			{
				return ImageResourceID::ApplicationRun;
			}
			wxSizer* GetWorkspaceSizer() const override
			{
				return m_MainSizer;
			}
			bool CanReload() const override
			{
				return true;
			}
		
			bool IsSubWorkspace() const override
			{
				return true;
			}
			size_t GetTabIndex() const override
			{
				return (size_t)TabIndex::Programs;
			}
	};
}
