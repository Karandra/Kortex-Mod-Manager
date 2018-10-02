#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxSingleton.h>
class KSaveManager;
class KSaveFile;
class KSaveManagerView;
class KPluginManager;

class KSaveManagerWorkspace: public KWorkspace, public KxSingletonPtr<KSaveManagerWorkspace>
{
	private:
		KProgramOptionUI m_SavesListViewOptions;
		KProgramOptionUI m_FileFiltersOptions;
		
		KSaveManager* m_Manager = NULL;
		std::unordered_set<wxString> m_ActiveFilters;

		wxBoxSizer* m_MainSizer = NULL;
		wxBoxSizer* m_ViewSizer = NULL;
		KxSplitterWindow* m_Splitter = NULL;
		KSaveManagerView* m_ViewModel = NULL;

	public:
		KSaveManagerWorkspace(KMainWindow* mainWindow, KSaveManager* manager);
		virtual ~KSaveManagerWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		void CreateViewPane();
		void CreateContextMenu(KxMenu& menu, const KSaveFile* saveEntry);

		bool FiltersMenu_IsAllFiltersActive() const;
		bool FiltersMenu_IsFilterActive(const wxString& filter) const
		{
			return m_ActiveFilters.count(filter);
		}
		void FiltersMenu_AllFiles(KxMenuEvent& event);
		void FiltersMenu_SpecificFilter(KxMenuEvent& event);

		void OnSyncPluginsList(const KSaveFile* saveEntry);
		void OnSavePluginsList(const KSaveFile* saveEntry);
		void OnRemoveSave(KSaveFile* saveEntry);

		virtual bool IsSubWorkspace() const override
		{
			return true;
		}
		virtual size_t GetTabIndex() const
		{
			return KWS_TABINDEX_SAVES;
		}
		
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetNameShort() const;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_JAR;
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
		void ProcessSelection(const KSaveFile* saveEntry);
		void ProcessContextMenu(const KSaveFile* saveEntry);
};
