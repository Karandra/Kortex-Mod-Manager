#pragma once
#include "stdafx.h"
#include "Utility/OperationWithProgress.h"
#include "Utility/TempFolderKeeper.h"
#include <KxFramework/KxArchiveEvent.h>

namespace Kortex
{
	class ModPackageProject;
	class GenericArchive;
}
namespace Kortex::PackageProject
{
	class FileItem;
}

namespace Kortex::PackageDesigner
{
	enum class BuildError
	{
		Success = 0,
		Generic,
		PackagePath,
	};

	class PackageBuilder: public Utility::TempFolderKeeper
	{
		friend class PackageBuilderOperation;

		private:
			const ModPackageProject& m_Project;
			Utility::OperationWithProgressBase& m_Thread;	
			wxString m_PackagePath;
			const bool m_BuildPreview = false;

			std::unique_ptr<GenericArchive> m_Archive;
			KxStringVector m_SourceFiles;
			KxStringVector m_ArchivePaths;
			KxStringVector m_MissingFiles;
			BuildError m_Status = BuildError::Generic;

		private:
			wxString GetTempFolder() const;
			wxString GetTempPackagePath() const;
			wxString GetImagePath(const wxString& fileName) const;
			wxString GetDocumentPath(const wxString& fileName) const;
			wxString GetFileDataEntryPath(const PackageProject::FileItem* fileDataEntry, const wxString& fileName) const;

			void SetPackagePath(const wxString& path)
			{
				m_PackagePath = path;
			}

		private:
			virtual void CheckProject();
			virtual void Configure();
			virtual void WritePackageConfig();
			virtual void ProcessInfo();
			virtual void ProcessInterface();
			virtual void ProcessFileData();

		public:
			PackageBuilder(const ModPackageProject& project, Utility::OperationWithProgressBase& thread, bool previewBuild = false);
			~PackageBuilder();

		public:
			BuildError GetStatus() const
			{
				return m_Status;
			}
			bool IsOK() const
			{
				return m_Status == BuildError::Success;
			}	
			KxStringVector GetMissingFiles() const
			{
				return m_MissingFiles;
			}
		
			const wxString& GetPackagePath() const;
			bool IsPrevievBuild() const
			{
				return m_BuildPreview;
			}

			bool Run();
	};
}

namespace Kortex::PackageDesigner
{
	class PackageBuilderOperation: public Utility::OperationWithProgressDialog<KxArchiveEvent>
	{
		private:
			const ModPackageProject& m_Project;
			wxString m_PackagePath;
			bool m_BuildPreview = false;

			KxStringVector m_MissingFiles;
			BuildError m_CheckStatus = BuildError::Generic;
			bool m_BuildOK = false;
			bool m_Cancelled = false;

		private:
			void EntryHandler();
			void OnEndHandler();

		public:
			PackageBuilderOperation(const ModPackageProject& project, bool previewBuild = false);

		public:
			void SetPrevievBuild(bool value)
			{
				m_BuildPreview = value;
			}
	};
}
