#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include "DisplayModel.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxPanel.h>

namespace Kortex::GameConfig
{
	class Workspace:
		public Application::DefaultWindowWorkspace<KxPanel>,
		public KxSingletonPtr<Workspace>
	{
		private:
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel m_DisplayModel;

			KxButton* m_SaveButton = nullptr;
			KxButton* m_DiscardButton = nullptr;

		private:
			void OnSaveButton(wxCommandEvent& event);
			void OnDiscardButton(wxCommandEvent& event);

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			Workspace();
			~Workspace();

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			ResourceID GetIcon() const override
			{
				return ImageResourceID::GearPencil;
			}

		public:
			void OnChangesMade();
			void OnChangesSaved();
			void OnChangesDiscarded();
	};
}
