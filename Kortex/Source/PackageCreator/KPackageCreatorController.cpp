#include "stdafx.h"
#include "KPackageCreatorController.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorBuilder.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "PackageProject/KPackageProjectDefs.h"
#include "PackageProject/KPackageProjectSerializerKMP.h"
#include "PackageProject/KPackageProjectSerializerFOMod.h"
#include "PackageManager/KModPackage.h"
#include "Archive/KArchive.h"
#include "KOperationWithProgress.h"
#include "KApp.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxTextFile.h>

wxString KPackageCreatorController::GetNewProjectName()
{
	return KTr("PackageCreator.NewProjectName");
}

KPackageCreatorController::KPackageCreatorController(KPackageCreatorWorkspace* workspace)
	:KWorkspaceController(workspace), m_Workspace(workspace)
{
}
KPackageCreatorController::~KPackageCreatorController()
{
}

wxString KPackageCreatorController::GetSaveConfirmationCaption() const
{
	return KTr("PackageCreator.SaveChanges.Caption");
}
wxString KPackageCreatorController::GetSaveConfirmationMessage() const
{
	return KTr("PackageCreator.SaveChanges.Message");
}

bool KPackageCreatorController::IsOK() const
{
	return KWorkspaceController::IsOK();
}
wxString KPackageCreatorController::GetProjectFileName() const
{
	wxString name = m_ProjectFile.AfterLast('\\');
	return name.IsEmpty() ? GetNewProjectName() : name;
}
wxString KPackageCreatorController::GetProjectName() const
{
	// Name -> ID -> translated name -> project file name
	if (m_Project)
	{
		const wxString& name = m_Project->GetInfo().GetName();
		if (!name.IsEmpty())
		{
			return name;
		}
		else
		{
			const wxString& id = m_Project->GetModID();
			if (!id.IsEmpty())
			{
				return id;
			}
			else
			{
				const wxString& translatedName = m_Project->GetInfo().GetTranslatedName();
				if (!translatedName.IsEmpty())
				{
					return translatedName;
				}
				else
				{
					wxString fileName = m_ProjectFile.AfterLast('\\').BeforeFirst('.');
					if (!fileName.IsEmpty())
					{
						return fileName;
					}
				}
			}
		}
	}
	return GetNewProjectName();
}

void KPackageCreatorController::ChangeNotify()
{
	m_HasChanges = true;
	m_Workspace->RefreshWindowTitleForProject();

	wxNotifyEvent event(KEVT_CONTROLLER_CHANGED);
	ProcessEvent(event);
}
void KPackageCreatorController::SaveChanges()
{
	KPackageProjectSerializerKMP serializer(true);
	serializer.Serialize(m_Project.get());
	KxTextFile::WriteToFile(m_ProjectFile, serializer.GetData());
	
	m_HasChanges = false;
	m_Workspace->RefreshWindowTitleForProject();

	wxNotifyEvent event(KEVT_CONTROLLER_SAVED);
	ProcessEvent(event);
}
void KPackageCreatorController::DiscardChanges()
{
	m_HasChanges = false;
	NewProject();

	wxNotifyEvent event(KEVT_CONTROLLER_DISCARDED);
	ProcessEvent(event);
}

void KPackageCreatorController::NewProject()
{
	m_ProjectFile.Clear();
	m_Project = std::make_unique<KPackageProject>();
	Reload();
}
void KPackageCreatorController::OpenProject(const wxString& filePath)
{
	m_ProjectFile = filePath;
	m_Project = std::make_unique<KPackageProject>();

	KPackageProjectSerializerKMP serializer(true);
	serializer.SetData(KxTextFile::ReadToString(filePath));
	serializer.Structurize(m_Project.get());

	Reload();
}
void KPackageCreatorController::SaveProject()
{
	SaveChanges();
}
void KPackageCreatorController::SaveProject(const wxString& filePath)
{
	m_ProjectFile = filePath;
	SaveChanges();
}
void KPackageCreatorController::ImportProjectFromPackage(const wxString& packagePath)
{
	m_ProjectFile.Clear();
	m_Project = std::make_unique<KPackageProject>();

	KModPackage(packagePath, *m_Project.get());
	m_Project->GetConfig().SetInstallPackageFile(packagePath);

	Reload();
}
void KPackageCreatorController::CreateProjectFromModEntry(const KModEntry& modEntry)
{
	m_ProjectFile.Clear();
	m_Project = std::make_unique<KPackageProject>();

	/* Info and config */
	m_Project->SetModID(modEntry.GetID());

	m_Project->GetConfig().SetInstallPackageFile(modEntry.GetInstallPackageFile());

	KPackageProjectInfo& info = m_Project->GetInfo();
	info.SetName(modEntry.GetName());
	info.SetVersion(modEntry.GetVersion());
	info.SetAuthor(modEntry.GetAuthor());
	info.SetDescription(modEntry.GetDescription());
	info.GetFixedWebSites() = modEntry.GetFixedWebSites();
	info.GetWebSites() = modEntry.GetWebSites();
	info.GetTags() = modEntry.GetTags();

	/* Interface */
	KPPIImageEntry& imageEntry = m_Project->GetInterface().GetImages().emplace_back(KPPIImageEntry());
	imageEntry.SetPath(modEntry.GetInfoFile());
	imageEntry.SetVisible(true);

	m_Project->GetInterface().SetMainImage(imageEntry.GetPath());

	Reload();
}
void KPackageCreatorController::ImportProject(KPackageProjectSerializer& serializer)
{
	m_ProjectFile.Clear();
	m_Project = std::make_unique<KPackageProject>();
	serializer.Structurize(m_Project.get());
	Reload();
}
void KPackageCreatorController::ExportProject(KPackageProjectSerializer& serializer)
{
	serializer.Serialize(m_Project.get());
}
void KPackageCreatorController::BuildProject(bool buildPreview)
{
	auto thread = new KPackageCreatorBuilderOperation(m_Project.get(), buildPreview);
	thread->SetDialogCaption(KTrf("PackageCreator.Build.Caption", GetProjectName()));
	thread->Run();
}

void KPackageCreatorController::Reload()
{
	ResetView();
	LoadView();
	m_Workspace->RefreshWindowTitleForProject();
}
void KPackageCreatorController::LoadView()
{
	m_Workspace->DoLoadAllPages();
}
void KPackageCreatorController::ResetView()
{
}
