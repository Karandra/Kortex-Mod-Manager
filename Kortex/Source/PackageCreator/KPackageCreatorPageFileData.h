#pragma once
#include "stdafx.h"
#include "KPackageCreatorPageBase.h"

namespace Kortex::PackageDesigner
{
	class KPackageCreatorWorkspace;
	class KPackageProjectFileData;
	class KPCFileDataMainListModel;
	class KPCFileDataFolderContentModel;
}

namespace Kortex::PackageDesigner
{
	class KPackageCreatorPageFileData: public KPackageCreatorPageBase
	{
		friend class KPackageCreatorWorkspace;

		private:
			KxSplitterWindow* m_Pane = nullptr;
			//KProgramOptionAI m_MainOptions;

			// Folders
			KxPanel* m_MainListPane = nullptr;
			KPCFileDataMainListModel* m_MainListModel = nullptr;
			//KProgramOptionAI m_MainListOptions;

			// Files
			KxPanel* m_FolderContentPane = nullptr;
			KPCFileDataFolderContentModel* m_ContentListModel = nullptr;
			//KProgramOptionAI m_ContentListModelOptions;

		private:
			void OnLoadProject(KPackageProjectFileData& projectFileData);
			KPackageProjectFileData& GetProjectFileData() const;

			void CreateMainListControls();
			void CreateFolderContentControls();

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;

		public:
			KPackageCreatorPageFileData(KPackageCreatorWorkspace& mainWorkspace, KPackageCreatorController& controller);
			~KPackageCreatorPageFileData();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Folder;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;

			KPCFileDataMainListModel* GetMainListModel() const
			{
				return m_MainListModel;
			}
			KPCFileDataFolderContentModel* GetFolderContentModel() const
			{
				return m_ContentListModel;
			}
	};
}
