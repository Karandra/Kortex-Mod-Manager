#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KRunManagerWorkspaceView;

class KRunManagerWorkspace: public KWorkspace, public KxSingletonPtr<KRunManagerWorkspace>
{
	private:
		KProgramOptionUI m_ProgramListViewOptions;

		wxBoxSizer* m_MainSizer = NULL;
		KRunManagerWorkspaceView* m_ViewModel = NULL;

	public:
		KRunManagerWorkspace(KMainWindow* mainWindow);
		virtual ~KRunManagerWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_APPLICATION_RUN;
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
