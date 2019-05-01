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
class KxSearchBox;
class KxHTMLWindow;

namespace Kortex
{
	class IGamePlugin;
}

namespace Kortex::PluginManager
{
	class PluginViewModel;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		public:
			static ImageResourceID GetStatusImageForPlugin(const IGamePlugin* plugin = nullptr);

		private:
			//KProgramOptionAI m_PluginListViewOptions;
			
			wxBoxSizer* m_MainSizer = nullptr;
			PluginViewModel* m_ModelView = nullptr;
			KxSearchBox* m_SearchBox = nullptr;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
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
			virtual ResourceID GetImageID() const override
			{
				return ImageResourceID::PlugDisconnect;
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
			void OnRunLootAPI(KxMenuEvent& event);

		public:
			PluginViewModel* GetModel() const
			{
				return m_ModelView;
			}
		
			void OnCreateViewContextMenu(KxMenu& menu, const IGamePlugin* plugin);
			void OnCreateSortingToolsMenu(KxMenu& menu, const IGamePlugin* plugin);
			void OnCreateImportExportMenu(KxMenu& menu, const IGamePlugin* plugin);

			void ProcessSelection(const IGamePlugin* plugin = nullptr);
			void HighlightPlugin(const IGamePlugin* plugin = nullptr);
	};
}
