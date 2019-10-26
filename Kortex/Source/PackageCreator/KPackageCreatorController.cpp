#include "stdafx.h"
#include "KPackageCreatorController.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorBuilder.h"
#include <Kortex/ModManager.hpp>
#include "PackageProject/KPackageProjectDefs.h"
#include "PackageProject/KPackageProjectSerializerKMP.h"
#include "PackageProject/KPackageProjectSerializerFOMod.h"
#include "ModPackages/ModPackage.h"
#include "Archive/KArchive.h"
#include "Utility/KOperationWithProgress.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxTextFile.h>

namespace Kortex::PackageDesigner
{
	wxString KPackageCreatorController::GetNewProjectName()
	{
		return KTr("PackageCreator.NewProjectName");
	}

	wxString KPackageCreatorController::GetSaveConfirmationCaption() const
	{
		return KTr("PackageCreator.SaveChanges.Caption");
	}
	wxString KPackageCreatorController::GetSaveConfirmationMessage() const
	{
		return KTr("PackageCreator.SaveChanges.Message");
	}
	bool KPackageCreatorController::QueryInterface(const KxIID& iid, void*& ptr) noexcept
	{
		return KxIObject::QueryAnyOf(iid, ptr, *this);
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
		m_Workspace.RefreshWindowTitleForProject();

		BroadcastProcessor::Get().ProcessEvent(EvtChanged);
	}
	void KPackageCreatorController::SaveChanges()
	{
		KPackageProjectSerializerKMP serializer(true);
		serializer.Serialize(m_Project.get());
		KxTextFile::WriteToFile(m_ProjectFile, serializer.GetData());

		m_HasChanges = false;
		m_Workspace.RefreshWindowTitleForProject();

		BroadcastProcessor::Get().ProcessEvent(EvtSaved);
	}
	void KPackageCreatorController::DiscardChanges()
	{
		m_HasChanges = false;
		NewProject();

		BroadcastProcessor::Get().ProcessEvent(EvtDiscarded);
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

		ModPackage(packagePath, *m_Project);
		m_Project->GetConfig().SetInstallPackageFile(packagePath);

		Reload();
	}
	void KPackageCreatorController::CreateProjectFromModEntry(const Kortex::IGameMod& modEntry)
	{
		m_ProjectFile.Clear();
		m_Project = std::make_unique<KPackageProject>();

		/* Info and config */
		m_Project->SetModID(modEntry.GetID());

		m_Project->GetConfig().SetInstallPackageFile(modEntry.GetPackageFile());

		KPackageProjectInfo& info = m_Project->GetInfo();
		info.SetName(modEntry.GetName());
		info.SetVersion(modEntry.GetVersion());
		info.SetAuthor(modEntry.GetAuthor());
		info.SetDescription(modEntry.GetDescription());
		info.GetModSourceStore() = modEntry.GetModSourceStore();
		info.GetTagStore() = modEntry.GetTagStore();

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
		m_Workspace.RefreshWindowTitleForProject();
	}
	void KPackageCreatorController::LoadView()
	{
		m_Workspace.DoLoadAllPages();
	}
	void KPackageCreatorController::ResetView()
	{
	}
}
