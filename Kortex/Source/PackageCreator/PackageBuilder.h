#pragma once
#include "stdafx.h"
#include "Archive/KArchive.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KTempFolderKeeper.h"

namespace Kortex
{
	class KPackageProject;
}
namespace Kortex::PackageProject
{
	class KPPFFileEntry;
}

namespace Kortex::PackageDesigner
{
	enum KPCBStatus
	{
		KPCB_STATUS_OK = 0,
		KPCB_STATUS_ERROR_GENERIC,
		KPCB_STATUS_ERROR_PACKAGE_PATH,
	};
	
	class PackageBuilder: public KTempFolderKeeper
	{
		friend class KPackageCreatorBuilderOperation;

		private:
			const KPackageProject* m_Project = nullptr;
			KOperationWithProgressBase* m_Thread = nullptr;	
			wxString m_PackagePath;
			const bool m_BuildPreview = false;

			KArchive m_Archive;
			KxStringVector m_SourceFiles;
			KxStringVector m_ArchivePaths;
			KxStringVector m_MissingFiles;
			KPCBStatus m_Status = KPCB_STATUS_ERROR_GENERIC;

		private:
			wxString GetTempFolder() const;
			wxString GetTempPackagePath() const;
			wxString GetImagePath(const wxString& fileName) const;
			wxString GetDocumentPath(const wxString& fileName) const;
			wxString GetFileDataEntryPath(const PackageProject::KPPFFileEntry* fileDataEntry, const wxString& fileName) const;

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
			PackageBuilder(const KPackageProject* project, KOperationWithProgressBase* thread, bool previewBuild = false);
			~PackageBuilder();

		public:
			KPCBStatus GetStatus() const
			{
				return m_Status;
			}
			bool IsOK() const
			{
				return m_Status == KPCB_STATUS_OK;
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
	class KPackageCreatorBuilderOperation: public KOperationWithProgressDialog<KxArchiveEvent>
	{
		private:
			const KPackageProject* m_Project = nullptr;
			bool m_BuildPreview = false;
			wxString m_PackagePath;

			KxStringVector m_MissingFiles;
			KPCBStatus m_CheckStatus = KPCB_STATUS_ERROR_GENERIC;
			bool m_BuildOK = false;
			bool m_Cancelled = false;

		private:
			void EntryHandler();
			void OnEndHandler();

		public:
			KPackageCreatorBuilderOperation(const KPackageProject* project, bool previewBuild = false);

		public:
			void SetPrevievBuild(bool value)
			{
				m_BuildPreview = value;
			}
	};
}
