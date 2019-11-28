#include "stdafx.h"
#include "WorkspaceDocument.h"
#include "Workspace.h"
#include "PackageBuilder.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include "PackageProject/Common.h"
#include "PackageProject/NativeSerializer.h"
#include "PackageProject/FOModSerializer.h"
#include "ModPackages/ModPackage.h"
#include "Archive/KArchive.h"
#include "Utility/KOperationWithProgress.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxTextFile.h>

namespace Kortex::PackageDesigner
{
	wxString WorkspaceDocument::GetNewProjectName()
	{
		return KTr("PackageCreator.NewProjectName");
	}

	wxString WorkspaceDocument::GetSaveConfirmationCaption() const
	{
		return KTr("PackageCreator.SaveChanges.Caption");
	}
	wxString WorkspaceDocument::GetSaveConfirmationMessage() const
	{
		return KTr("PackageCreator.SaveChanges.Message");
	}

	bool WorkspaceDocument::QueryInterface(const KxIID& iid, void*& ptr) noexcept
	{
		return UseAnyOf<IWorkspace>(iid, ptr, m_Workspace) || QuerySelf(iid, ptr, *this);
	}

	wxString WorkspaceDocument::GetProjectFileName() const
	{
		wxString name = m_ProjectFile.AfterLast('\\');
		return name.IsEmpty() ? GetNewProjectName() : name;
	}
	wxString WorkspaceDocument::GetProjectName() const
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

	void WorkspaceDocument::ChangeNotify()
	{
		m_HasChanges = true;
		m_Workspace.RefreshWindowTitleForProject();

		BroadcastProcessor::Get().ProcessEvent(EvtChanged);
	}
	void WorkspaceDocument::SaveChanges()
	{
		PackageProject::NativeSerializer serializer(true);
		serializer.Serialize(m_Project.get());
		KxTextFile::WriteToFile(m_ProjectFile, serializer.GetData());

		m_HasChanges = false;
		m_Workspace.RefreshWindowTitleForProject();

		BroadcastProcessor::Get().ProcessEvent(EvtSaved);
	}
	void WorkspaceDocument::DiscardChanges()
	{
		m_HasChanges = false;
		NewProject();

		BroadcastProcessor::Get().ProcessEvent(EvtDiscarded);
	}

	void WorkspaceDocument::NewProject()
	{
		m_ProjectFile.Clear();
		m_Project = std::make_unique<ModPackageProject>();
		Reload();
	}
	void WorkspaceDocument::OpenProject(const wxString& filePath)
	{
		m_ProjectFile = filePath;
		m_Project = std::make_unique<ModPackageProject>();

		PackageProject::NativeSerializer serializer(true);
		serializer.SetData(KxTextFile::ReadToString(filePath));
		serializer.Structurize(m_Project.get());

		Reload();
	}
	void WorkspaceDocument::SaveProject()
	{
		SaveChanges();
	}
	void WorkspaceDocument::SaveProject(const wxString& filePath)
	{
		m_ProjectFile = filePath;
		SaveChanges();
	}
	void WorkspaceDocument::ImportProjectFromPackage(const wxString& packagePath)
	{
		m_ProjectFile.Clear();
		m_Project = std::make_unique<ModPackageProject>();

		ModPackage(packagePath, *m_Project);
		m_Project->GetConfig().SetInstallPackageFile(packagePath);

		Reload();
	}
	void WorkspaceDocument::CreateProjectFromModEntry(const Kortex::IGameMod& modEntry)
	{
		m_ProjectFile.Clear();
		m_Project = std::make_unique<ModPackageProject>();

		/* Info and config */
		m_Project->SetModID(modEntry.GetID());

		m_Project->GetConfig().SetInstallPackageFile(modEntry.GetPackageFile());

		PackageProject::InfoSection& info = m_Project->GetInfo();
		info.SetName(modEntry.GetName());
		info.SetVersion(modEntry.GetVersion());
		info.SetAuthor(modEntry.GetAuthor());
		info.SetDescription(modEntry.GetDescription());
		info.GetModSourceStore() = modEntry.GetModSourceStore();
		info.GetTagStore() = modEntry.GetTagStore();

		/* Interface */
		PackageProject::ImageItem& imageEntry = m_Project->GetInterface().GetImages().emplace_back(PackageProject::ImageItem());
		imageEntry.SetPath(modEntry.GetInfoFile());
		imageEntry.SetVisible(true);

		m_Project->GetInterface().SetMainImage(imageEntry.GetPath());

		Reload();
	}
	void WorkspaceDocument::ImportProject(PackageProject::Serializer& serializer)
	{
		m_ProjectFile.Clear();
		m_Project = std::make_unique<ModPackageProject>();
		serializer.Structurize(m_Project.get());
		Reload();
	}
	void WorkspaceDocument::ExportProject(PackageProject::Serializer& serializer)
	{
		serializer.Serialize(m_Project.get());
	}
	void WorkspaceDocument::BuildProject(bool buildPreview)
	{
		auto thread = new KPackageCreatorBuilderOperation(m_Project.get(), buildPreview);
		thread->SetDialogCaption(KTrf("PackageCreator.Build.Caption", GetProjectName()));
		thread->Run();
	}

	void WorkspaceDocument::Reload()
	{
		ResetView();
		LoadView();
		m_Workspace.RefreshWindowTitleForProject();
	}
	void WorkspaceDocument::LoadView()
	{
		m_Workspace.DoLoadAllPages();
	}
	void WorkspaceDocument::ResetView()
	{
	}
}
