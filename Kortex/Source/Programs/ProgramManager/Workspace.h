#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include <Kortex/Events.hpp>
#include <KxFramework/KxSingleton.h>

namespace Kortex::ProgramManager
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
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

			void OnVFSToggled(Kortex::VirtualFileSystemEvent& event);

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const override;

			KImageEnum GetImageID() const override
			{
				return KIMG_APPLICATION_RUN;
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
