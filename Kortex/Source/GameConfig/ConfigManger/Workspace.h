#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "DisplayModel.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex::GameConfig
{
	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel m_DisplayModel;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			virtual bool OnOpenWorkspace() override;
			virtual bool OnCloseWorkspace() override;
			virtual void OnReloadWorkspace() override;

		public:
			virtual wxString GetID() const override;
			virtual wxString GetName() const override;
			virtual wxString GetNameShort() const;
			virtual KImageEnum GetImageID() const override
			{
				return KIMG_GEAR_PENCIL;
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
}
