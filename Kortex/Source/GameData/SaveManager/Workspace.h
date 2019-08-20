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
			std::unordered_set<wxString> m_ActiveFilters;

			wxBoxSizer* m_MainSizer = nullptr;
			wxBoxSizer* m_ViewSizer = nullptr;
			KxSplitterWindow* m_Splitter = nullptr;
			DisplayModel* m_DisplayModel = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			~Workspace();
			bool OnCreateWorkspace() override;

		private:
			void CreateViewPane();
			void UpdateFilters();

			bool FiltersMenu_IsAllFiltersActive() const;
			bool FiltersMenu_IsFilterActive(const wxString& filter) const
			{
				return m_ActiveFilters.count(filter);
			}
			void FiltersMenu_AllFiles(KxMenuEvent& event);
			void FiltersMenu_SpecificFilter(KxMenuEvent& event);

			void OnSyncPluginsList(const IBethesdaGameSave& save);
			void OnSavePluginsList(const IBethesdaGameSave& save);
			bool OnRemoveSave(IGameSave& save);

			bool IsSubWorkspace() const override
			{
				return true;
			}
			size_t GetTabIndex() const
			{
				return (size_t)TabIndex::Saves;
			}
		
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const;
			ResourceID GetImageID() const override
			{
				return ImageResourceID::Jar;
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
			void OnSelection(const IGameSave* save);
			void OnContextMenu(const IGameSave* save);
	};
}
