#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxPanel.h>

namespace Kortex
{
	class VirtualFSEvent;
}

namespace Kortex::ProgramManager
{
	class DisplayModel;

	class Workspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<Workspace>
	{
		private:
			BroadcastReciever m_BroadcastReciever;
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel* m_ViewModel = nullptr;

		private:
			void OnMainFSToggled(VirtualFSEvent& event);

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
				return ImageResourceID::ApplicationRun;
			}
	};
}
