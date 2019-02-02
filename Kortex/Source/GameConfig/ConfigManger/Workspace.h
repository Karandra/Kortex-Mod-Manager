#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxSingleton.h>
#include "Items/CategoryItem.h"

namespace KxDataView2
{
	class View;
}

namespace Kortex
{
	class IGameConfigManager;
}

namespace Kortex::GameConfig
{
	class DefaultGameConfigManager;

	class Workspace: public KWorkspace, public KxSingletonPtr<Workspace>
	{
		friend class DefaultGameConfigManager;

		private:
			IGameConfigManager& m_Manager;
			const ITranslator& m_Translator;

			wxBoxSizer* m_MainSizer = nullptr;
			KxDataView2::View* m_View = nullptr;
			std::unordered_map<wxString, CategoryItem> m_Categories;

		public:
			Workspace(KMainWindow* mainWindow);
			virtual ~Workspace();
			virtual bool OnCreateWorkspace() override;

		private:
			virtual bool OnOpenWorkspace() override;
			virtual bool OnCloseWorkspace() override;
			virtual void OnReloadWorkspace() override;

			void ClearView();
			void LoadView();

		public:
			virtual wxString GetID() const override;
			virtual wxString GetName() const override;
			virtual wxString GetNameShort() const;
			virtual KImageEnum GetImageID() const override
			{
				return KIMG_GEAR_PENCIL;
			}
			virtual wxSizer* GetWorkspaceSizer() const override
			{
				return m_MainSizer;
			}
			virtual bool CanReload() const override
			{
				return true;
			}
	};
}
