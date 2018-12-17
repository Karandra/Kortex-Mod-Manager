#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include <Kortex/Events.hpp>
#include <KxFramework/KxSingleton.h>
class KxSearchBox;

namespace Kortex::VirtualGameFolder
{
	class DisplayModel;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			wxBoxSizer* m_MainSizer = nullptr;
			DisplayModel* m_Model = nullptr;
			KxSearchBox* m_SearchBox = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			virtual bool OnOpenWorkspace() override;
			virtual bool OnCloseWorkspace() override;
			virtual void OnReloadWorkspace() override;

			void OnModSerach(wxCommandEvent& event);
			void OnViewInvalidated(IEvent& event);

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
				return (size_t)TabIndex::VirtualGameFolder;
			}
	};
}
