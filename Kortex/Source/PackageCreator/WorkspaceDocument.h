#pragma once
#include "stdafx.h"
#include "Application/IWorkspaceDocument.h"
#include "PackageProject/ModPackageProject.h"

namespace Kortex
{
	class ModPackage;
	class IGameMod;
}
namespace Kortex::PackageProject
{
	class Serializer;
}

namespace Kortex::PackageDesigner
{
	class Workspace;

	class WorkspaceDocument: public KxRTTI::ExtendInterface<WorkspaceDocument, IWorkspaceDocument>
	{
		public:
			static wxString GetNewProjectName();

		private:
			Workspace& m_Workspace;
			std::unique_ptr<ModPackageProject> m_Project = nullptr;
			wxString m_ProjectFile;
			bool m_HasChanges = false;

		protected:
			wxString GetSaveConfirmationCaption() const override;
			wxString GetSaveConfirmationMessage() const override;

		public:
			WorkspaceDocument(Workspace& workspace)
				:m_Workspace(workspace)
			{
			}

		public:
			using KxIObject::QueryInterface;
			bool QueryInterface(const KxIID& iid, void*& ptr) noexcept override;

			ModPackageProject* GetProject() const
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
			void CreateProjectFromModEntry(const IGameMod& modEntry);
			void ImportProject(PackageProject::Serializer& serializer);
			void ExportProject(PackageProject::Serializer& serializer);
			void BuildProject(bool buildPreview = false);

			void Reload();
			void LoadView();
			void ResetView();
	};
}
