#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxPanel.h>
class KxSearchBox;

namespace Kortex::VirtualGameFolder
{
	class DisplayModel;
	class Workspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<Workspace>
	{
		private:
			BroadcastReciever m_BroadcastReciever;

			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel* m_DisplayModel = nullptr;
			KxSearchBox* m_SearchBox = nullptr;

		private:
			void OnModSerach(wxCommandEvent& event);
			void OnViewInvalidated(BroadcastEvent& event);
			void OnBeginReload(BroadcastEvent& event);
			void OnEndReload(BroadcastEvent& event);

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			~Workspace();

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Folders;
			}
			IWorkspaceContainer* GetPreferredContainer() const override;
	};
}
