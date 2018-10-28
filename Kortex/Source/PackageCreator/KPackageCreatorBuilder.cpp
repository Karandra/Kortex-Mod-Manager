#include "stdafx.h"
#include "KPackageCreatorBuilder.h"
#include "KOperationWithProgress.h"
#include "PackageProject/KPackageProject.h"
#include "PackageProject/KPackageProjectSerializerKMP.h"
#include "PackageProject/KPackageProjectSerializerFOMod.h"
#include "InstallWizard/KInstallWizardDialog.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxTaskDialog.h>

#define TestContinue()		if (!m_Thread->CanContinue()) { return; }

wxString KPackageCreatorBuilder::GetTempPackagePath() const
{
	return GetPackagePath() + ".tmp";
}
wxString KPackageCreatorBuilder::GetTempFolder() const
{
	return wxString::Format("%s\\%s\\0x%p\\", wxFileName::GetTempDir(), KApp::Get().GetAppName(), this);
}
wxString KPackageCreatorBuilder::GetImagePath(const wxString& fileName) const
{
	return KPackageProjectSerializer::GetDefaultFOModRoot() + "\\Images\\" + fileName.AfterLast('\\');
}
wxString KPackageCreatorBuilder::GetDocumentPath(const wxString& fileName) const
{
	return KPackageProjectSerializer::GetDefaultFOModRoot() + "\\Documents\\" + fileName.AfterLast('\\');
}
wxString KPackageCreatorBuilder::GetFileDataEntryPath(const KPPFFileEntry* fileDataEntry, const wxString& fileName) const
{
	return fileDataEntry->GetID() + "\\" + fileName;
}

void KPackageCreatorBuilder::CheckProject()
{
	const KPackageProjectInfo& info = m_Project->GetInfo();
	const KPackageProjectInterface& interfaceConfig = m_Project->GetInterface();
	const KPackageProjectFileData& fileData = m_Project->GetFileData();

	m_Status = KPCB_STATUS_OK;
	auto CheckAndAddMissingFile = [this](const wxString& path)
	{
		if (!KxFile(path).IsFileExist())
		{
			m_Status = KPCB_STATUS_ERROR_GENERIC;
			m_MissingFiles.push_back(path);
		}
	};
	auto CheckAndAddMissingFolder = [this](const wxString& path)
	{
		if (!KxFile(path).IsFolderExist())
		{
			m_Status = KPCB_STATUS_ERROR_GENERIC;
			m_MissingFiles.push_back(path);
		}
	};

	// Check temporary and real package paths
	if (!wxFileName(GetTempPackagePath()).IsOk() || !wxFileName(GetPackagePath()).IsOk())
	{
		m_Status = KPCB_STATUS_ERROR_PACKAGE_PATH;
		return;
	}

	// Check documents
	for (const KLabeledValue& entry: info.GetDocuments())
	{
		TestContinue();
		CheckAndAddMissingFile(entry.GetValue());
	}

	// Check images
	for (const KPPIImageEntry& entry: interfaceConfig.GetImages())
	{
		TestContinue();
		CheckAndAddMissingFile(entry.GetPath());
	}

	// Check files
	for (const auto& fileEntry: fileData.GetData())
	{
		TestContinue();
		if (const KPPFFolderEntry* folderEntry = fileEntry->ToFolderEntry())
		{
			CheckAndAddMissingFolder(folderEntry->GetSource());
			for (const auto& entry: folderEntry->GetFiles())
			{
				TestContinue();
				CheckAndAddMissingFile(entry.GetSource());
			}
		}
		else
		{
			CheckAndAddMissingFile(fileEntry->GetSource());
		}
	}
}
void KPackageCreatorBuilder::Configure()
{
	auto ConvertMethod = [](const wxString& methodName)
	{
		if (methodName == "LZMA")
		{
			return KArchiveNS::Method::LZMA;
		}
		if (methodName == "LZMA2")
		{
			return KArchiveNS::Method::LZMA2;
		}
		if (methodName == "PPMd")
		{
			return KArchiveNS::Method::PPMd;
		}
		if (methodName == "BZip2")
		{
			return KArchiveNS::Method::BZip2;
		}
		return KArchiveNS::Method::LZMA;
	};

	const KPackageProjectConfig& config = m_Project->GetConfig();

	m_Archive.SetPropertyBool(KArchiveNS::PropertyBool::Solid, config.IsSolidArchive());
	m_Archive.SetPropertyBool(KArchiveNS::PropertyBool::Solid, config.IsMultithreadingUsed());
	m_Archive.SetPropertyInt(KArchiveNS::PropertyInt::Method, ConvertMethod(config.GetCompressionMethod()));
	m_Archive.SetPropertyInt(KArchiveNS::PropertyInt::CompressionLevel, config.GetCompressionLevel());
	m_Archive.SetPropertyInt(KArchiveNS::PropertyInt::DictionarySize, config.GetCompressionDictionarySize());
}
void KPackageCreatorBuilder::WritePackageConfig()
{
	// Create KMP config
	{
		wxString packageConfigFile = CreateTempFile();
		KPackageProjectSerializerKMP serializer(false);
		serializer.SetPackageDataRoot(KPackageProjectSerializer::GetDefaultFOModRoot());
		serializer.Serialize(m_Project);
		KxTextFile::WriteToFile(packageConfigFile, serializer.GetData());

		m_SourceFiles.push_back(packageConfigFile);
		m_ArchivePaths.push_back("KortexPackage.xml");
	}

	// Create FOMod config
	{
		wxString infoFile = CreateTempFile();
		wxString sModuleConfigFile = CreateTempFile();

		KPackageProjectSerializerFOMod serializer;
		serializer.ExportToNativeFormat(true);
		serializer.SetPackageDataRoot(KPackageProjectSerializer::GetDefaultFOModRoot());
		serializer.Serialize(m_Project);
		KxTextFile::WriteToFile(infoFile, serializer.GetInfoXML());
		KxTextFile::WriteToFile(sModuleConfigFile, serializer.GetModuleConfigXML());

		m_SourceFiles.push_back(infoFile);
		m_ArchivePaths.push_back("FOMod\\Info.xml");
		
		m_SourceFiles.push_back(sModuleConfigFile);
		m_ArchivePaths.push_back("FOMod\\ModuleConfig.xml");
	}
}
void KPackageCreatorBuilder::ProcessInfo()
{
	const KPackageProjectInfo& info = m_Project->GetInfo();
	for (const KLabeledValue& entry: info.GetDocuments())
	{
		TestContinue();

		m_SourceFiles.push_back(entry.GetValue());
		m_ArchivePaths.push_back(GetDocumentPath(entry.GetValue()));
	}
}
void KPackageCreatorBuilder::ProcessInterface()
{
	const KPackageProjectInterface& interfaceConfig = m_Project->GetInterface();
	for (const KPPIImageEntry& entry: interfaceConfig.GetImages())
	{
		TestContinue();

		m_SourceFiles.push_back(entry.GetPath());
		m_ArchivePaths.push_back(GetImagePath(entry.GetPath()));
	}
}
void KPackageCreatorBuilder::ProcessFileData()
{
	if (IsPrevievBuild())
	{
		return;
	}

	const KPackageProjectFileData& fileData = m_Project->GetFileData();
	for (const auto& fileEntry: fileData.GetData())
	{
		TestContinue();

		if (const KPPFFolderEntry* folderEntry = fileEntry->ToFolderEntry())
		{
			for (const auto& folderItem: folderEntry->GetFiles())
			{
				TestContinue();

				m_SourceFiles.push_back(folderItem.GetSource());
				m_ArchivePaths.push_back(GetFileDataEntryPath(folderEntry, folderItem.GetDestination()));
			}
		}
		else
		{
			m_SourceFiles.push_back(fileEntry->GetSource());
			m_ArchivePaths.push_back(fileEntry->GetID());
		}
	}
}

KPackageCreatorBuilder::KPackageCreatorBuilder(const KPackageProject* project, KOperationWithProgressBase* thread, bool previewBuild)
	:m_Project(project), m_Thread(thread), m_BuildPreview(previewBuild)
{
	m_Thread->LinkHandler(&m_Archive, KxEVT_ARCHIVE);
}
KPackageCreatorBuilder::~KPackageCreatorBuilder()
{
}

bool KPackageCreatorBuilder::Run()
{
	CheckProject();
	if (IsOK())
	{
		// Create new package as *.tmp file to avoid accidentally overwrite existing package
		wxString tempPackagePath = GetTempPackagePath();

		// Create containing folder
		KxFile(wxFileName(tempPackagePath).GetPath()).CreateFolder();

		if (m_Archive.Open(tempPackagePath))
		{
			Configure();
			WritePackageConfig();
			ProcessInfo();
			ProcessInterface();
			ProcessFileData();

			// Compress now
			if (m_Archive.CompressSpecifiedFiles(m_SourceFiles, m_ArchivePaths))
			{
				// Compression successful, rename temp file to real file
				KxFile(tempPackagePath).Rename(GetPackagePath(), true);
				return true;
			}
			else
			{
				// Archive is incorrect if compression was aborted
				KxFile(tempPackagePath).RemoveFile();
			}
		}
	}
	return false;
}

const wxString& KPackageCreatorBuilder::GetPackagePath() const
{
	return m_PackagePath.IsEmpty() ? m_Project->GetConfig().GetInstallPackageFile() : m_PackagePath;
}

//////////////////////////////////////////////////////////////////////////
void KPackageCreatorBuilderOperation::EntryHandler()
{
	KPackageCreatorBuilder builder(m_Project, this, m_BuildPreview);
	if (m_BuildPreview)
	{
		builder.SetPackagePath(m_PackagePath);
	}

	m_BuildOK = builder.Run();
	m_CheckStatus = builder.GetStatus();
	if (m_CheckStatus != KPCB_STATUS_OK)
	{
		m_MissingFiles = builder.GetMissingFiles();
	}
}
void KPackageCreatorBuilderOperation::OnEndHandler()
{
	if (m_CheckStatus != KPCB_STATUS_OK)
	{
		wxString message;
		switch (m_CheckStatus)
		{
			case KPCB_STATUS_ERROR_PACKAGE_PATH:
			{
				message = KTr("PackageCreator.Build.CheckError.PackagePath");
				break;
			}
			default:
			{
				message = KTr("PackageCreator.Build.CheckError.MissingFiles");
				break;
			}
		};

		KxTaskDialog dialog(KApp::Get().GetTopWindow(), KxID_NONE, KTr("PackageCreator.Build.CheckError"), message, KxBTN_OK, KxICON_WARNING);
		dialog.SetExMessage(KxString::Join(m_MissingFiles, "\r\n"));
		dialog.ShowModal();
	}
	else if (!m_Cancelled)
	{
		if (m_BuildOK)
		{
			if (m_BuildPreview)
			{
				KInstallWizardDialog* dialog = new KInstallWizardDialog();
				dialog->SetOptionEnabled(KIWD_OPTION_PREVIEW);
				dialog->SetOptionEnabled(KIWD_OPTION_CLEANUP);
				dialog->Create(KApp::Get().GetTopWindow(), m_PackagePath);
			}
			else
			{
				wxString path = m_Project->GetConfig().GetInstallPackageFile();
				wxString size = KxFile(path).GetFormattedFileSize(2);
				wxString info = wxString::Format("%s: \"%s\"\r\n%s: %s", KTr(KxID_FILE), path, KTr("Generic.Size"), size);
				KxTaskDialog(KApp::Get().GetTopWindow(), KxID_NONE, KTr("PackageCreator.Build.Complete"), info, KxBTN_OK, KxICON_INFORMATION).ShowModal();
			}
		}
		else
		{
			KxTaskDialog(KApp::Get().GetTopWindow(), KxID_NONE, KTr("PackageCreator.Build.BuildError"), wxEmptyString, KxBTN_OK, KxICON_ERROR).ShowModal();
		}
	}
}

KPackageCreatorBuilderOperation::KPackageCreatorBuilderOperation(const KPackageProject* project, bool previewBuild)
	:KOperationWithProgressDialog(true, NULL), m_Project(project), m_BuildPreview(previewBuild)
{
	if (previewBuild)
	{
		m_PackagePath = KTempFolderKeeper::CreateGlobalTempFile(".kmp");
	}

	OnRun([this](KOperationWithProgressBase* self)
	{
		EntryHandler();
	});
	OnEnd([this](KOperationWithProgressBase* self)
	{
		OnEndHandler();
	});
	OnCancel([this](KOperationWithProgressBase* self)
	{
		m_Cancelled = true;
	});
}
