#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "DisplayModel.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxButton.h>

namespace Kortex::GameConfig
{
	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel m_DisplayModel;

			KxButton* m_SaveButton = nullptr;
			KxButton* m_DiscardButton = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			~Workspace();

		private:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

			void OnSaveButton(wxCommandEvent& event);
			void OnDiscardButton(wxCommandEvent& event);

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const;
			ResourceID GetImageID() const override
			{
				return ImageResourceID::GearPencil;
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
			void OnChangesMade();
			void OnChangesSaved();
			void OnChangesDiscarded();
	};
}
