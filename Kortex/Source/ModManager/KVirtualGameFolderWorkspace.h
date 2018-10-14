#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KProgramOptions.h"
#include "KEventsFwd.h"
#include <KxFramework/KxSingleton.h>
class KxSearchBox;
class KVirtualGameFolderModel;

class KVirtualGameFolderWorkspace: public KWorkspace, public KxSingletonPtr<KVirtualGameFolderWorkspace>
{
	private:
		KProgramOptionUI m_OptionsUI;
		KProgramOptionUI m_ViewOptions;

		wxBoxSizer* m_MainSizer = NULL;
		KVirtualGameFolderModel* m_Model = NULL;
		KxSearchBox* m_SearchBox = NULL;

	public:
		KVirtualGameFolderWorkspace(KMainWindow* mainWindow);
		virtual ~KVirtualGameFolderWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

		void OnModSerach(wxCommandEvent& event);
		void OnViewInvalidated(KEvent& event);

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetNameShort() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_FOLDERS;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		
		virtual bool IsSubWorkspace() const
		{
			return true;
		}
		virtual bool CanReload() const override
		{
			return true;
		}
};
