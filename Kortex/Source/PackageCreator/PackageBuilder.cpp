#include "stdafx.h"
#include "PackageBuilder.h"
#include "PackageProject/ModPackageProject.h"
#include "PackageProject/Serializer/NativeSerializer.h"
#include "PackageProject/Serializer/FOModSerializer.h"
#include "Archive/GenericArchive.h"
#include "Utility/OperationWithProgress.h"
#include <Kortex/Application.hpp>
#include <Kortex/InstallWizard.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxTaskDialog.h>

#define TestContinue()		if (!m_Thread.CanContinue()) { return; }

namespace Kortex::PackageDesigner
{
	wxString PackageBuilder::GetTempPackagePath() const
	{
		return GetPackagePath() + ".tmp";
	}
	wxString PackageBuilder::GetTempFolder() const
	{
		return wxString::Format("%s\\%s\\0x%p\\", wxFileName::GetTempDir(), Kortex::IApplication::GetInstance()->GetName(), this);
	}
	wxString PackageBuilder::GetImagePath(const wxString& fileName) const
	{
		return PackageProject::Serializer::GetDefaultFOModRoot() + "\\Images\\" + fileName.AfterLast('\\');
	}
	wxString PackageBuilder::GetDocumentPath(const wxString& fileName) const
	{
		return PackageProject::Serializer::GetDefaultFOModRoot() + "\\Documents\\" + fileName.AfterLast('\\');
	}
	wxString PackageBuilder::GetFileDataEntryPath(const PackageProject::FileItem* fileDataEntry, const wxString& fileName) const
	{
		return fileDataEntry->GetID() + "\\" + fileName;
	}

	void PackageBuilder::CheckProject()
	{
		const PackageProject::InfoSection& info = m_Project.GetInfo();
		const PackageProject::InterfaceSection& interfaceConfig = m_Project.GetInterface();
		const PackageProject::FileDataSection& fileData = m_Project.GetFileData();

		m_Status = BuildError::Success;
		auto CheckAndAddMissingFile = [this](const wxString& path)
		{
			if (!KxFile(path).IsFileExist())
			{
				m_Status = BuildError::Generic;
				m_MissingFiles.push_back(path);
			}
		};
		auto CheckAndAddMissingFolder = [this](const wxString& path)
		{
			if (!KxFile(path).IsFolderExist())
			{
				m_Status = BuildError::Generic;
				m_MissingFiles.push_back(path);
			}
		};

		// Check temporary and real package paths
		if (!wxFileName(GetTempPackagePath()).IsOk() || !wxFileName(GetPackagePath()).IsOk())
		{
			m_Status = BuildError::PackagePath;
			return;
		}

		// Check documents
		for (const Utility::LabeledValue& entry: info.GetDocuments())
		{
			TestContinue();
			CheckAndAddMissingFile(entry.GetValue());
		}

		// Check images
		for (const PackageProject::ImageItem& entry: interfaceConfig.GetImages())
		{
			TestContinue();
			CheckAndAddMissingFile(entry.GetPath());
		}

		// Check files
		for (const auto& fileEntry: fileData.GetItems())
		{
			TestContinue();
			if (const PackageProject::FolderItem* folderEntry; fileEntry->QueryInterface(folderEntry))
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
	void PackageBuilder::Configure()
	{
		auto ConvertMethod = [](const wxString& methodName)
		{
			if (methodName == wxS("LZMA"))
			{
				return Archive::Method::LZMA;
			}
			if (methodName == wxS("LZMA2"))
			{
				return Archive::Method::LZMA2;
			}
			if (methodName == wxS("PPMd"))
			{
				return Archive::Method::PPMd;
			}
			if (methodName == wxS("BZip2"))
			{
				return Archive::Method::BZip2;
			}
			return Archive::Method::LZMA2;
		};

		const PackageProject::ConfigSection& config = m_Project.GetConfig();

		m_Archive->SetProperty(KxArchive::Property::Compression_Format, Archive::Format::SevenZip);
		m_Archive->SetProperty(KxArchive::Property::Compression_Solid, config.IsSolidArchive());
		m_Archive->SetProperty(KxArchive::Property::Compression_MultiThreaded, config.IsMultithreadingUsed());
		m_Archive->SetProperty(KxArchive::Property::Compression_Method, ConvertMethod(config.GetCompressionMethod()));
		m_Archive->SetProperty(KxArchive::Property::Compression_Level, config.GetCompressionLevel());
		m_Archive->SetProperty(KxArchive::Property::Compression_DictionarySize, config.GetCompressionDictionarySize());
	}
	void PackageBuilder::WritePackageConfig()
	{
		// Create KMP config
		{
			wxString packageConfigFile = CreateTempFile();
			PackageProject::NativeSerializer serializer(false);
			serializer.SetPackageDataRoot(PackageProject::Serializer::GetDefaultFOModRoot());
			serializer.Serialize(m_Project);
			KxTextFile::WriteToFile(packageConfigFile, serializer.GetData());

			m_SourceFiles.push_back(packageConfigFile);
			m_ArchivePaths.emplace_back("KortexPackage.xml");
		}

		// Create FOMod config
		{
			wxString infoFile = CreateTempFile();
			wxString moduleConfigFile = CreateTempFile();

			PackageProject::FOModSerializer serializer;
			serializer.ExportToNativeFormat(true);
			serializer.SetPackageDataRoot(PackageProject::Serializer::GetDefaultFOModRoot());
			serializer.Serialize(m_Project);
			KxTextFile::WriteToFile(infoFile, serializer.GetInfoXML());
			KxTextFile::WriteToFile(moduleConfigFile, serializer.GetModuleConfigXML());

			m_SourceFiles.push_back(infoFile);
			m_ArchivePaths.emplace_back("FOMod\\Info.xml");

			m_SourceFiles.push_back(moduleConfigFile);
			m_ArchivePaths.emplace_back("FOMod\\ModuleConfig.xml");
		}
	}
	void PackageBuilder::ProcessInfo()
	{
		const PackageProject::InfoSection& info = m_Project.GetInfo();
		for (const Utility::LabeledValue& entry: info.GetDocuments())
		{
			TestContinue();

			m_SourceFiles.push_back(entry.GetValue());
			m_ArchivePaths.push_back(GetDocumentPath(entry.GetValue()));
		}
	}
	void PackageBuilder::ProcessInterface()
	{
		const PackageProject::InterfaceSection& interfaceConfig = m_Project.GetInterface();
		for (const PackageProject::ImageItem& entry: interfaceConfig.GetImages())
		{
			TestContinue();

			m_SourceFiles.push_back(entry.GetPath());
			m_ArchivePaths.push_back(GetImagePath(entry.GetPath()));
		}
	}
	void PackageBuilder::ProcessFileData()
	{
		if (IsPrevievBuild())
		{
			return;
		}

		const PackageProject::FileDataSection& fileData = m_Project.GetFileData();
		for (const auto& fileEntry: fileData.GetItems())
		{
			TestContinue();

			if (const PackageProject::FolderItem* folderEntry; fileEntry->QueryInterface(folderEntry))
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

	PackageBuilder::PackageBuilder(const ModPackageProject& project, Utility::OperationWithProgressBase& thread, bool previewBuild)
		:m_Project(project), m_Thread(thread), m_BuildPreview(previewBuild)
	{
	}
	PackageBuilder::~PackageBuilder()
	{
	}

	bool PackageBuilder::Run()
	{
		CheckProject();
		if (IsOK())
		{
			// Create new package as *.tmp file to avoid accidentally overwrite existing package
			wxString tempPackagePath = GetTempPackagePath();

			// Create containing folder
			KxFile(wxFileName(tempPackagePath).GetPath()).CreateFolder();

			m_Archive = std::make_unique<GenericArchive>(tempPackagePath);
			m_Thread.LinkHandler(m_Archive.get(), KxArchiveEvent::EvtProcess);

			Configure();
			WritePackageConfig();
			ProcessInfo();
			ProcessInterface();
			ProcessFileData();

			// Compress now
			if (m_Archive->CompressSpecifiedFiles(m_SourceFiles, m_ArchivePaths))
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
		return false;
	}

	const wxString& PackageBuilder::GetPackagePath() const
	{
		return m_PackagePath.IsEmpty() ? m_Project.GetConfig().GetInstallPackageFile() : m_PackagePath;
	}
}

namespace Kortex::PackageDesigner
{
	void PackageBuilderOperation::EntryHandler()
	{
		PackageBuilder builder(m_Project, *this, m_BuildPreview);
		if (m_BuildPreview)
		{
			builder.SetPackagePath(m_PackagePath);
		}

		m_BuildOK = builder.Run();
		m_CheckStatus = builder.GetStatus();
		if (m_CheckStatus != BuildError::Success)
		{
			m_MissingFiles = builder.GetMissingFiles();
		}
	}
	void PackageBuilderOperation::OnEndHandler()
	{
		if (m_CheckStatus != BuildError::Success)
		{
			wxString message;
			switch (m_CheckStatus)
			{
				case BuildError::PackagePath:
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

			KxTaskDialog dialog(IApplication::GetInstance()->GetTopWindow(), KxID_NONE, KTr("PackageCreator.Build.CheckError"), message, KxBTN_OK, KxICON_WARNING);
			dialog.SetExMessage(KxString::Join(m_MissingFiles, "\r\n"));
			dialog.ShowModal();
		}
		else if (!m_Cancelled)
		{
			if (m_BuildOK)
			{
				if (m_BuildPreview)
				{
					using namespace InstallWizard;

					WizardDialog* dialog = new WizardDialog();
					dialog->SetOptionEnabled(DialogOptions::Debug);
					dialog->SetOptionEnabled(DialogOptions::Cleanup);
					dialog->Create(IApplication::GetInstance()->GetTopWindow(), m_PackagePath);
				}
				else
				{
					wxString path = m_Project.GetConfig().GetInstallPackageFile();
					wxString size = KxFile(path).GetFormattedFileSize(2);
					wxString info = wxString::Format("%s: \"%s\"\r\n%s: %s", KTr(KxID_FILE), path, KTr("Generic.Size"), size);
					KxTaskDialog(IApplication::GetInstance()->GetTopWindow(), KxID_NONE, KTr("PackageCreator.Build.Complete"), info, KxBTN_OK, KxICON_INFORMATION).ShowModal();
				}
			}
			else
			{
				KxTaskDialog(IApplication::GetInstance()->GetTopWindow(), KxID_NONE, KTr("PackageCreator.Build.BuildError"), wxEmptyString, KxBTN_OK, KxICON_ERROR).ShowModal();
			}
		}
	}

	PackageBuilderOperation::PackageBuilderOperation(const ModPackageProject& project, bool previewBuild)
		:OperationWithProgressDialog(true, nullptr), m_Project(project), m_BuildPreview(previewBuild)
	{
		if (previewBuild)
		{
			m_PackagePath = Utility::TempFolderKeeper::CreateGlobalTempFile(".kmp");
		}

		OnRun([this]()
		{
			EntryHandler();
		});
		OnEnd([this]()
		{
			OnEndHandler();
		});
		OnCancel([this]()
		{
			m_Cancelled = true;
		});
	}
}
