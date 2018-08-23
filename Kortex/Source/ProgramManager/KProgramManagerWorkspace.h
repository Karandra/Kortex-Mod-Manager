#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KProgramManagerModel;

class KProgramManagerWorkspace: public KWorkspace, public KxSingletonPtr<KProgramManagerWorkspace>
{
	private:
		KProgramOptionUI m_ProgramListViewOptions;

		wxBoxSizer* m_MainSizer = NULL;
		KProgramManagerModel* m_ViewModel = NULL;

	public:
		KProgramManagerWorkspace(KMainWindow* mainWindow);
		virtual ~KProgramManagerWorkspace();
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
