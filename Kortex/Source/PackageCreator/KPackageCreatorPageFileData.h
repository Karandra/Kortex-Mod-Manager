#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorPageBase.h"
class KPackageCreatorWorkspace;
class KPackageProjectFileData;
class KPCFileDataMainListModel;
class KPCFileDataFolderContentModel;

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

	public:
		KPackageCreatorPageFileData(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller);
		virtual ~KPackageCreatorPageFileData();

	private:
		virtual bool OnCreateWorkspace() override;
		KPackageProjectFileData& GetProjectFileData() const;

		void CreateMainListControls();
		void CreateFolderContentControls();

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnLoadProject(KPackageProjectFileData& projectFileData);

	public:
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_FOLDER;
		}
		virtual wxString GetID() const override;
		virtual wxString GetPageName() const override;

		KPCFileDataMainListModel* GetMainListModel() const
		{
			return m_MainListModel;
		}
		KPCFileDataFolderContentModel* GetFolderContentModel() const
		{
			return m_ContentListModel;
		}
};
