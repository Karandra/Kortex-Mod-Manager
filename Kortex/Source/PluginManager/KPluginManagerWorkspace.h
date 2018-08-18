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
class KPluginManager;
class KPluginManagerBethesdaGeneric;
class KPluginManagerListModel;
class KPMPluginEntry;
class KPluginManagerConfigSortingToolEntry;

class KxSearchBox;
class KxHTMLWindow;

class KPluginManagerWorkspace: public KWorkspace, public KxSingletonPtr<KPluginManagerWorkspace>
{
	public:
		static KImageEnum GetStatusImageForPlugin(const KPMPluginEntry* pluginEntry);

	private:
		KProgramOptionUI m_PluginListViewOptions;
		KPluginManagerBethesdaGeneric* m_Manager = NULL;

		wxBoxSizer* m_MainSizer = NULL;
		KPluginManagerListModel* m_ModelView = NULL;
		KxSearchBox* m_SearchBox = NULL;

	public:
		KPluginManagerWorkspace(KMainWindow* mainWindow, KPluginManagerBethesdaGeneric* manager);
		virtual ~KPluginManagerWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		void CreateModelView();

		virtual bool IsSubWorkspace() const override
		{
			return true;
		}
		virtual size_t GetTabIndex() const
		{
			return KWS_TABINDEX_PLUGINS;
		}
		
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;
		
		void OnModSerach(wxCommandEvent& event);

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetNameShort() const;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_PLUG_DISCONNECT;
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
		void UpdatePluginTypeCounter(KxMenuItem* item);

	public:
		KPluginManagerListModel* GetModel() const
		{
			return m_ModelView;
		}
		
		void OnCreateViewContextMenu(KxMenu& menu, const KPMPluginEntry* entry);
		void OnCreateSortingToolsMenu(KxMenu& menu, const KPMPluginEntry* entry);
		void OnCreateImportExportMenu(KxMenu& menu, const KPMPluginEntry* entry);

		void ProcessSelection(const KPMPluginEntry* entry = NULL);
		void HighlightPlugin(const KPMPluginEntry* entry = NULL);
};
