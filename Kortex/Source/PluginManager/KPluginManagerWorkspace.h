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
class KPluginViewBaseModel;
class KPluginEntry;

class KxSearchBox;
class KxHTMLWindow;

class KPluginManagerWorkspace: public KWorkspace, public KxSingletonPtr<KPluginManagerWorkspace>
{
	public:
		static KImageEnum GetStatusImageForPlugin(const KPluginEntry* pluginEntry = NULL);

	private:
		KProgramOptionUI m_PluginListViewOptions;

		wxBoxSizer* m_MainSizer = NULL;
		KPluginViewBaseModel* m_ModelView = NULL;
		KxSearchBox* m_SearchBox = NULL;

	public:
		KPluginManagerWorkspace(KMainWindow* mainWindow);
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
			return (size_t)TabIndex::Plugins;
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
		KPluginViewBaseModel* GetModel() const
		{
			return m_ModelView;
		}
		
		void OnCreateViewContextMenu(KxMenu& menu, const KPluginEntry* entry);
		void OnCreateSortingToolsMenu(KxMenu& menu, const KPluginEntry* entry);
		void OnCreateImportExportMenu(KxMenu& menu, const KPluginEntry* entry);

		void ProcessSelection(const KPluginEntry* entry = NULL);
		void HighlightPlugin(const KPluginEntry* entry = NULL);
};
