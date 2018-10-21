#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "KProgramOptions.h"
#include "KEventsFwd.h"
#include <KxFramework/KxSingleton.h>
class KProgramManagerModel;

class KProgramWorkspace: public KWorkspace, public KxSingletonPtr<KProgramWorkspace>
{
	private:
		KProgramOptionUI m_ProgramListViewOptions;

		wxBoxSizer* m_MainSizer = NULL;
		KProgramManagerModel* m_ViewModel = NULL;

	public:
		KProgramWorkspace(KMainWindow* mainWindow);
		virtual ~KProgramWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

		void OnVFSToggled(KVFSEvent& event);

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetNameShort() const override;

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
		
		virtual bool IsSubWorkspace() const override
		{
			return true;
		}
		virtual size_t GetTabIndex() const override
		{
			return (size_t)TabIndex::Programs;
		}
};
