#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IGameSave;
	class ISaveManager;
}

namespace Kortex::SaveManager
{
	class IBethesdaGameSave;
	class DisplayModel;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		private:
			ISaveManager* m_Manager = nullptr;
			std::unordered_set<wxString> m_ActiveFilters;

			wxBoxSizer* m_MainSizer = nullptr;
			wxBoxSizer* m_ViewSizer = nullptr;
			KxSplitterWindow* m_Splitter = nullptr;
			DisplayModel* m_DisplayModel = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			void CreateViewPane();
			void CreateContextMenu(KxMenu& menu, const IGameSave* saveEntry);

			bool FiltersMenu_IsAllFiltersActive() const;
			bool FiltersMenu_IsFilterActive(const wxString& filter) const
			{
				return m_ActiveFilters.count(filter);
			}
			void FiltersMenu_AllFiles(KxMenuEvent& event);
			void FiltersMenu_SpecificFilter(KxMenuEvent& event);

			void OnSyncPluginsList(const IBethesdaGameSave* saveEntry);
			void OnSavePluginsList(const IBethesdaGameSave* saveEntry);
			void OnRemoveSave(IGameSave* saveEntry);

			virtual bool IsSubWorkspace() const override
			{
				return true;
			}
			virtual size_t GetTabIndex() const
			{
				return (size_t)TabIndex::Saves;
			}
		
			virtual bool OnOpenWorkspace() override;
			virtual bool OnCloseWorkspace() override;
			virtual void OnReloadWorkspace() override;

		public:
			virtual wxString GetID() const override;
			virtual wxString GetName() const override;
			virtual wxString GetNameShort() const;
			virtual ResourceID GetImageID() const override
			{
				return ImageResourceID::Jar;
			}
			virtual wxSizer* GetWorkspaceSizer() const override
			{
				return m_MainSizer;
			}
			virtual bool CanReload() const override
			{
				return false;
			}

		private:
			void LoadData();

		public:
			void ProcessSelection(const IGameSave* saveEntry);
			void ProcessContextMenu(const IGameSave* saveEntry);
	};
}
