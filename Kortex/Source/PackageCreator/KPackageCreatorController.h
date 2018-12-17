#pragma once
#include "stdafx.h"
#include "UI/KWorkspaceController.h"
#include "PackageProject/KPackageProject.h"
class KPackageCreatorWorkspace;
class KPackageManager;
class KPackageProjectSerializer;
class KModPackage;
enum KPPPackageType;

namespace Kortex
{
	class IGameMod;
}

class KPackageCreatorController: public KWorkspaceController
{
	public:
		static wxString GetNewProjectName();

	private:
		KPackageCreatorWorkspace* m_Workspace = nullptr;
		std::unique_ptr<KPackageProject> m_Project = nullptr;
		wxString m_ProjectFile;
		bool m_HasChanges = false;

	public:
		KPackageCreatorController(KPackageCreatorWorkspace* workspace);
		virtual ~KPackageCreatorController();

	protected:
		virtual wxString GetSaveConfirmationCaption() const override;
		virtual wxString GetSaveConfirmationMessage() const override;

	public:
		bool IsOK() const override;
		KPackageManager* GetManager() const
		{
			return nullptr;
		}
		KPackageProject* GetProject() const
		{
			return m_Project.get();
		}
		const wxString& GetProjectFilePath() const
		{
			return m_ProjectFile;
		}
		wxString GetProjectFileName() const;
		wxString GetProjectName() const;
		bool HasProjectFilePath() const
		{
			return !m_ProjectFile.IsEmpty();
		}

		void ChangeNotify();
		bool HasUnsavedChanges() const override
		{
			return m_HasChanges;
		}
		void SaveChanges() override;
		void DiscardChanges() override;

		void NewProject();
		void OpenProject(const wxString& filePath);
		void SaveProject();
		void SaveProject(const wxString& filePath);
		void ImportProjectFromPackage(const wxString& packagePath);
		void CreateProjectFromModEntry(const Kortex::IGameMod& modEntry);
		void ImportProject(KPackageProjectSerializer& serializer);
		void ExportProject(KPackageProjectSerializer& serializer);
		void BuildProject(bool buildPreview = false);

		virtual void Reload() override;
		virtual void LoadView() override;

	private:
		virtual void ResetView() override;
};
