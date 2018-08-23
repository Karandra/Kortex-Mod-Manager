#include "stdafx.h"
#include "KPackageManagerListModel.h"
#include "KPackageManager.h"
#include "KModPackage.h"
#include "UI/KMainWindow.h"
#include "Profile/KProfile.h"
#include "InstallWizard/KInstallWizardDialog.h"
#include "PackageCreator/KPackageCreatorWorkspace.h"
#include "KOperationWithProgress.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileBrowseDialog.h>

enum ColumnID
{
	Name,
	ModificationDate,
	Type,
	Size,
};
enum MenuID
{
	Open,
	OpenLocation,
	ImportProject,
	ExtractFiles,

	Remove,
	Rename,

	Properties,
};

void KPackageManagerListModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KPackageManagerListModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPackageManagerListModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPackageManagerListModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});

	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 450);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, 125);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 150);
}

void KPackageManagerListModel::GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	if (row < m_Data.size())
	{
		const KxFileFinderItem& item = m_Data[row];
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				data = item.GetName();
				break;
			}
		};
	}
}
void KPackageManagerListModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	if (row < m_Data.size())
	{
		const KxFileFinderItem& item = m_Data[row];
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				data = KxDataViewBitmapTextValue(item.GetName(), GetIcon(item));
				break;
			}
			case ColumnID::ModificationDate:
			{
				data = KAux::FormatDateTime(item.GetModificationTime());
				break;
			}
			case ColumnID::Type:
			{
				data = GetType(item);
				break;
			}
			case ColumnID::Size:
			{
				data = item.IsFile() ? KxFile::FormatFileSize(item.GetFileSize(), 2) : wxEmptyString;
				break;
			}
		};
	}
}
bool KPackageManagerListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	if (row < m_Data.size())
	{
		KxFileFinderItem& item = m_Data[row];
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString newName = data.As<wxString>();
				if (KxFile(item.GetFullPath()).Rename(item.GetSource() + '\\' + newName, false))
				{
					item.SetName(newName);
					item.UpdateInfo();
					return true;
				}
				else
				{
					wxBell();
				}
				break;
			}
		};
	}
	return false;
}

void KPackageManagerListModel::OnSelectItem(KxDataViewEvent& event)
{
	ClearInfo();

	const KxFileFinderItem* entry = GetDataEntry(event.GetItem());
	if (entry && entry->IsFile())
	{
		KMainWindow::GetInstance()->SetStatus(entry->GetName());
		if (CanAutoShowPackageInfo())
		{
			auto thread = new KPackageManagerListModelThread(entry->GetFullPath(), this);
			thread->SetDialogCaption(entry->GetName());
			thread->Run();
		}
	}
}
void KPackageManagerListModel::OnActivateItem(KxDataViewEvent& event)
{
	const KxFileFinderItem* entry = GetDataEntry(event.GetItem());
	if (entry)
	{
		if (entry->IsDirectory())
		{
			Navigate(entry->GetFullPath());
		}
		else
		{
			if (IsPackageOK())
			{
				RunInstallWizard(*entry);
			}
			else
			{
				KInstallWizardDialog::ShowInvalidPackageDialog(GetViewTLW(), entry->GetFullPath());
			}
		}
	}
	else
	{
		Navigate(m_PrevPath);
	}
}
void KPackageManagerListModel::OnContextMenu(KxDataViewEvent& event)
{
	const KxFileFinderItem* entry = GetDataEntry(event.GetItem());
	KxDataViewColumn* column = event.GetColumn();
	if (entry && column)
	{
		m_ContextMenu_Open->Enable(entry->IsFile() && IsPackageOK());
		m_ContextMenu_ImportProject->Enable(IsPackageOK());
		m_ContextMenu_ExtractFiles->Enable(entry->IsFile());

		switch (m_ContextMenu.Show(GetView()))
		{
			case MenuID::Open:
			{
				RunInstallWizard(*entry);
				break;
			}
			case MenuID::OpenLocation:
			{
				if (entry->IsDirectory())
				{
					KxShell::Execute(GetViewTLW(), entry->GetFullPath(), "open");
				}
				else
				{
					KxShell::OpenFolderAndSelectItem(entry->GetFullPath());
				}
				break;
			}
			case MenuID::ImportProject:
			{
				KPackageCreatorWorkspace::GetInstance()->CreateNow();
				KPackageCreatorWorkspace::GetInstance()->ImportProjectFromPackage(entry->GetFullPath());
				KPackageCreatorWorkspace::GetInstance()->SwitchHere();
				break;
			}
			case MenuID::ExtractFiles:
			{
				KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN_FOLDER);
				if (dialog.ShowModal() == KxID_OK)
				{
					wxString outPath = dialog.GetResult();
					if (IsPackageOK())
					{
						outPath += '\\' + KAux::MakeSafeFileName(m_Package->GetName());
					}
					else
					{
						outPath += '\\' + entry->GetName();
					}

					KPackageManager::ExtractAcrhiveThreaded(GetViewTLW(), entry->GetFullPath(), outPath);
				}
				break;
			}

			case MenuID::Remove:
			{
				if (KxShell::FileOperation(entry->GetFullPath(), KxFS_FILE, KxFOF_DELETE, true, false, GetViewTLW()))
				{
					ItemDeleted(KxDataViewItem(), event.GetItem());
				}
				break;
			}
			case MenuID::Rename:
			{
				GetView()->EditItem(event.GetItem(), column);
				break;
			}

			case MenuID::Properties:
			{
				KxShell::Execute(GetViewTLW(), entry->GetFullPath(), "properties");
				break;
			}
		};
	}
}

void KPackageManagerListModel::CreateContextMenu()
{
	m_ContextMenu_Open = m_ContextMenu.Add(new KxMenuItem(MenuID::Open, T(KxID_OPEN)));
	m_ContextMenu_Open->SetDefault();

	m_ContextMenu.Add(new KxMenuItem(MenuID::OpenLocation, T("MainMenu.OpenLocation")))->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
	m_ContextMenu_ImportProject = m_ContextMenu.Add(new KxMenuItem(MenuID::ImportProject, T("ModManager.Menu.Package.ImportProject")));
	m_ContextMenu_ExtractFiles = m_ContextMenu.Add(new KxMenuItem(MenuID::ExtractFiles, T("ModManager.Menu.Package.Extract")));

	m_ContextMenu.AddSeparator();
	m_ContextMenu.Add(new KxMenuItem(MenuID::Remove, T(KxID_REMOVE)));
	m_ContextMenu.Add(new KxMenuItem(MenuID::Rename, T(KxID_RENAME)));

	m_ContextMenu.AddSeparator();
	m_ContextMenu.Add(new KxMenuItem(MenuID::Properties, T(KxID_PROPERTIES)));
}
void KPackageManagerListModel::RunInstallWizard(const KxFileFinderItem& entry)
{
	KInstallWizardDialog* installWizard = new KInstallWizardDialog();
	if (m_Package && m_Package->IsOK())
	{
		installWizard->Create(GetViewTLW(), std::move(m_Package));
	}
	else
	{
		installWizard->Create(GetViewTLW(), entry.GetFullPath());
	}
}
wxBitmap KPackageManagerListModel::GetIcon(const KxFileFinderItem& entry) const
{
	wxBitmap icon;
	if (entry.IsDirectory())
	{
		icon = KGetBitmap(KIMG_FOLDER);
	}
	else if (KAux::IsSingleFileExtensionMatches(entry.GetName(), "kmp"))
	{
		icon = KGetBitmap(KIMG_APPLICATION_LOGO_SMALL);
	}
	else if (KAux::IsSingleFileExtensionMatches(entry.GetName(), "smi"))
	{
		icon = KGetBitmap(KIMG_SKSM_LOGO_SMALL);
	}
	else if (KAux::IsSingleFileExtensionMatches(entry.GetName(), "7z"))
	{
		icon = KGetBitmap(KIMG_7ZIP);
	}
	else if (KAux::IsFileExtensionMatches(entry.GetName(), KPackageManager::GetSuppoptedExtensions()))
	{
		icon = KGetBitmap(KIMG_FOLDER_ZIPPER);
	}
	else
	{
		icon = KGetBitmap(KIMG_DOCUMENT);
	}
	return icon;
}
wxString KPackageManagerListModel::GetType(const KxFileFinderItem& entry) const
{
	return entry.IsFile() ? KxShell::GetTypeName(entry.GetName().AfterLast('.')) : T(KxID_FOLDER);
}

KPackageManagerListModel::KPackageManagerListModel()
{
	CreateContextMenu();
	SetDataViewFlags(KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
}

size_t KPackageManagerListModel::GetItemCount() const
{
	if (!m_CurrentPath.IsEmpty())
	{
		return m_Data.size();
	}
	return 0;
}
void KPackageManagerListModel::Navigate(const wxString& sNavigatePath)
{
	m_CurrentPath = sNavigatePath;
	m_PrevPath = m_CurrentPath.BeforeLast('\\');
	m_IsRoot = m_CurrentPath == GetHomePath();

	m_Data.clear();
	
	// Folders
	{
		KxFileFinder tFinder(m_CurrentPath, "*");
		KxFileFinderItem item = tFinder.FindNext();
		while (item.IsOK())
		{
			if (item.IsNormalItem() && item.IsDirectory())
			{
				m_Data.push_back(item);
			}
			item = tFinder.FindNext();
		}
	}

	// Files
	{
		KxFileFinder tFinder(m_CurrentPath, "*");
		KxFileFinderItem item = tFinder.FindNext();
		while (item.IsOK())
		{
			if (item.IsNormalItem() && item.IsFile())
			{
				m_Data.push_back(item);
			}
			item = tFinder.FindNext();
		}
	}
	RefreshItems();

	// Not the best place but whatever
	m_AutoShowPackageInfo = KPackageManager::Get().GetOptions().GetAttributeBool("AutoShowPackageInfo", true);
}
void KPackageManagerListModel::NavigateUp()
{
	if (!m_PrevPath.IsEmpty() && !m_IsRoot)
	{
		Navigate(m_PrevPath);
	}
}
void KPackageManagerListModel::NavigateHome()
{
	Navigate(GetHomePath());
	m_PrevPath.clear();
	m_IsRoot = true;
}
void KPackageManagerListModel::Search(const wxString& mask)
{
	m_Data.clear();
	m_PrevPath.clear();
	m_IsRoot = true;

	std::function<void(const wxString&)> Recurse = [this, &Recurse, &mask](const wxString& path)
	{
		KxFileFinder finder(path, "*");
		KxFileFinderItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsNormalItem())
			{
				if (item.IsFile() && KAux::CheckSearchMask(mask, item.GetName()))
				{
					m_Data.push_back(item);
				}
				else
				{
					Recurse(item.GetFullPath());
				}
			}
			item = finder.FindNext();
		}
	};
	Recurse(GetHomePath());
	RefreshItems();
}

void KPackageManagerListModel::LoadInfo()
{
	if (IsPackageOK())
	{
		// Name
		KMainWindow::GetInstance()->SetStatus(m_Package->GetName(), 0, KIMG_TICK_CIRCLE_FRAME);

		// Logo
		const KPPIImageEntry* pLogo = m_Package->GetLogoImage();
		m_ImageView->SetBitmap(pLogo ? pLogo->GetBitmap() : wxNullBitmap);

		// Description
		m_Description->SetTextValue(m_Package->GetDescription());
		m_Description->Enable(true);
	}
}
void KPackageManagerListModel::ClearInfo()
{
	m_ImageView->SetBitmap(wxNullBitmap);
	m_Description->Clear();
	m_Description->Enable(false);
	KMainWindow::GetInstance()->ClearStatus();
}

//////////////////////////////////////////////////////////////////////////
KPackageManagerListModelThread::KPackageManagerListModelThread(const wxString& filePath, KPackageManagerListModel* model)
	:KOperationWithProgressDialog(true, model->GetView()), m_Model(model)
{
	OnRun([this, filePath](KOperationWithProgressBase* self)
	{
		m_Model->m_Package = std::make_unique<KModPackage>();

		LinkHandler(&GetPackage().GetArchive(), KxEVT_ARCHIVE);
		GetPackage().Create(filePath);
	});
	OnEnd([this](KOperationWithProgressBase* self)
	{
		if (m_Model->IsPackageOK())
		{
			m_Model->LoadInfo();
		}
		m_Model->GetViewTLW()->Raise();
		m_Model->GetView()->Enable();
		m_Model->GetView()->SetFocus();
	});
}
