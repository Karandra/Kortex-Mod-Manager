#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxMenu.h>
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

	class Workspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<Workspace>
	{
		private:
			std::unordered_set<wxString> m_ActiveFilters;

			wxBoxSizer* m_MainSizer = nullptr;
			wxBoxSizer* m_ViewSizer = nullptr;
			KxSplitterWindow* m_Splitter = nullptr;
			DisplayModel* m_DisplayModel = nullptr;

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			~Workspace();

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

		public:
			wxString GetID() const override;
			wxString GetName() const override;
			wxString GetNameShort() const override;
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Jar;
			}

		public:
			void OnSelection(const IGameSave* save);
			void OnContextMenu(const IGameSave* save);
	};
}
