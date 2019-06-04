#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/PackageManager.hpp>
#include <Kortex/InstallWizard.hpp>
#include "DisplayModel.h"
#include "ModPackages/IPackageManager.h"
#include "ModPackages/ModPackage.h"
#include "UI/KMainWindow.h"
#include "PackageCreator/KPackageCreatorWorkspace.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KAux.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileBrowseDialog.h>

using namespace Kortex;
using namespace Kortex::Application;

namespace
{
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
}

namespace Kortex::PackageManager
{
	void DisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
		{
			KxMenu menu;
			if (GetView()->CreateColumnSelectionMenu(menu))
			{
				GetView()->OnColumnSelectionMenu(menu);
			}
		});

		KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;
		{
			auto info = GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 450, flags);
			info.GetColumn()->SortAscending();
		}
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125, flags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, 125, flags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 150, flags);
	}

	void DisplayModel::GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
	{
		if (row < m_Data.size())
		{
			const KxFileItem& item = m_Data[row];
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
	void DisplayModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
	{
		if (row < m_Data.size())
		{
			const KxFileItem& item = m_Data[row];
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
	bool DisplayModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
	{
		if (row < m_Data.size())
		{
			KxFileItem& item = m_Data[row];
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
	bool DisplayModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
	{
		const KxFileItem& item1 = m_Data[row1];
		const KxFileItem& item2 = m_Data[row2];

		// Folders first
		if (item1.IsFile() != item2.IsFile())
		{
			return item1.IsFile() < item2.IsFile();
		}

		switch (column ? column->GetID() : ColumnID::Name)
		{
			case ColumnID::Name:
			{
				return KxComparator::IsEqual(item1.GetName(), item2.GetName());
			}
			case ColumnID::Size:
			{
				if (item1.IsFile() && item2.IsFile())
				{
					return item1.GetFileSize() < item2.GetFileSize();
				}
			}
			case ColumnID::Type:
			{
				return KxComparator::IsEqual(GetType(item1), GetType(item2));
			}
			case ColumnID::ModificationDate:
			{
				return item1.GetModificationTime() < item2.GetModificationTime();
			}
		};
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		ClearInfo();

		const KxFileItem* entry = GetDataEntry(event.GetItem());
		if (entry && entry->IsFile())
		{
			KMainWindow::GetInstance()->SetStatus(entry->GetName());
			if (CanAutoShowPackageInfo())
			{
				auto thread = new DisplayModelExtractionOperation(entry->GetFullPath(), this);
				thread->SetDialogCaption(entry->GetName());
				thread->Run();
			}
		}
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		const KxFileItem* entry = GetDataEntry(event.GetItem());
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
					InstallWizard::WizardDialog::ShowInvalidPackageDialog(GetViewTLW(), entry->GetFullPath());
				}
			}
		}
		else
		{
			Navigate(m_PrevPath);
		}
	}
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
		const KxFileItem* entry = GetDataEntry(event.GetItem());
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

						IPackageManager::ExtractAcrhiveWithProgress(GetViewTLW(), entry->GetFullPath(), outPath);
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

	void DisplayModel::CreateContextMenu()
	{
		m_ContextMenu_Open = m_ContextMenu.Add(new KxMenuItem(MenuID::Open, KTr(KxID_OPEN)));
		m_ContextMenu_Open->SetDefault();

		m_ContextMenu.Add(new KxMenuItem(MenuID::OpenLocation, KTr("MainMenu.OpenLocation")))->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderOpen));
		m_ContextMenu_ImportProject = m_ContextMenu.Add(new KxMenuItem(MenuID::ImportProject, KTr("ModManager.Menu.Package.ImportProject")));
		m_ContextMenu_ExtractFiles = m_ContextMenu.Add(new KxMenuItem(MenuID::ExtractFiles, KTr("ModManager.Menu.Package.Extract")));

		m_ContextMenu.AddSeparator();
		m_ContextMenu.Add(new KxMenuItem(MenuID::Remove, KTr(KxID_REMOVE)));
		m_ContextMenu.Add(new KxMenuItem(MenuID::Rename, KTr(KxID_RENAME)));

		m_ContextMenu.AddSeparator();
		m_ContextMenu.Add(new KxMenuItem(MenuID::Properties, KTr(KxID_PROPERTIES)));
	}
	void DisplayModel::RunInstallWizard(const KxFileItem& entry)
	{
		InstallWizard::WizardDialog* installWizard = new InstallWizard::WizardDialog();
		if (m_Package && m_Package->IsOK())
		{
			installWizard->Create(GetViewTLW(), std::move(m_Package));
		}
		else
		{
			installWizard->Create(GetViewTLW(), entry.GetFullPath());
		}
	}
	wxBitmap DisplayModel::GetIcon(const KxFileItem& entry) const
	{
		wxBitmap icon;
		if (entry.IsDirectory())
		{
			icon = ImageProvider::GetBitmap(ImageResourceID::Folder);
		}
		else if (KAux::IsSingleFileExtensionMatches(entry.GetName(), "kmp"))
		{
			icon = ImageProvider::GetBitmap(ImageResourceID::KortexLogoSmall);
		}
		else if (KAux::IsSingleFileExtensionMatches(entry.GetName(), "smi"))
		{
			icon = ImageProvider::GetBitmap(ImageResourceID::SKSMLogoSmall);
		}
		else
		{
			icon = KxShell::GetFileIcon(entry.GetFullPath(), true);
			if (!icon.IsOk())
			{
				icon = ImageProvider::GetBitmap(ImageResourceID::Document);
			}
		}
		return icon;
	}
	wxString DisplayModel::GetType(const KxFileItem& entry) const
	{
		return entry.IsFile() ? KxShell::GetTypeName(entry.GetName().AfterLast('.')) : KTr(KxID_FOLDER);
	}

	DisplayModel::DisplayModel()
	{
		CreateContextMenu();
		SetDataViewFlags(KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
	}

	size_t DisplayModel::GetItemCount() const
	{
		if (!m_CurrentPath.IsEmpty())
		{
			return m_Data.size();
		}
		return 0;
	}
	void DisplayModel::Navigate(const wxString& sNavigatePath)
	{
		m_CurrentPath = sNavigatePath;
		m_PrevPath = m_CurrentPath.BeforeLast('\\');
		m_IsRoot = m_CurrentPath == GetHomePath();

		m_Data.clear();

		// Folders
		{
			KxFileFinder tFinder(m_CurrentPath, "*");
			KxFileItem item = tFinder.FindNext();
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
			KxFileItem item = tFinder.FindNext();
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
		m_AutoShowPackageInfo = GetGlobalOptionOf<IPackageManager>(OName::Packages).GetAttributeBool(OName::AutoShowInfo, true);
	}
	void DisplayModel::NavigateUp()
	{
		if (!m_PrevPath.IsEmpty() && !m_IsRoot)
		{
			Navigate(m_PrevPath);
		}
	}
	void DisplayModel::NavigateHome()
	{
		Navigate(GetHomePath());
		m_PrevPath.clear();
		m_IsRoot = true;
	}
	void DisplayModel::Search(const wxString& mask)
	{
		m_Data.clear();
		m_PrevPath.clear();
		m_IsRoot = true;

		wxString searchMask;
		KAux::SetSearchMask(searchMask, mask);

		std::function<void(const wxString&)> Recurse = [this, &Recurse, &searchMask](const wxString& path)
		{
			KxFileFinder finder(path, "*");
			KxFileItem item = finder.FindNext();
			while (item.IsOK())
			{
				if (item.IsNormalItem())
				{
					if (item.IsFile() && KAux::CheckSearchMask(searchMask, item.GetName()))
					{
						m_Data.push_back(item);
					}
					else if (item.IsDirectory())
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

	void DisplayModel::LoadInfo()
	{
		if (IsPackageOK())
		{
			// Name
			KMainWindow::GetInstance()->SetStatus(m_Package->GetName(), 0, ImageResourceID::TickCircleFrame);

			// Logo
			const KPPIImageEntry* pLogo = m_Package->GetLogoImage();
			m_ImageView->SetBitmap(pLogo ? pLogo->GetBitmap() : wxNullBitmap);

			// Description
			m_Description->SetTextValue(m_Package->GetDescription());
			m_Description->Enable(true);
		}
	}
	void DisplayModel::ClearInfo()
	{
		m_ImageView->SetBitmap(wxNullBitmap);
		m_Description->Clear();
		m_Description->Enable(false);
		KMainWindow::GetInstance()->ClearStatus();
	}

	bool DisplayModel::IsPackageOK() const
	{
		return m_Package != nullptr && m_Package->IsOK();
	}
}

namespace Kortex::PackageManager
{
	DisplayModelExtractionOperation::DisplayModelExtractionOperation(const wxString& filePath, DisplayModel* model)
		:KOperationWithProgressDialog(true, model->GetView()), m_DisplayModel(model)
	{
		OnRun([this, filePath](KOperationWithProgressBase* self)
		{
			m_DisplayModel->m_Package = std::make_unique<ModPackage>();

			LinkHandler(&GetPackage().GetArchive(), KxEVT_ARCHIVE);
			GetPackage().Create(filePath);
		});
		OnEnd([this](KOperationWithProgressBase* self)
		{
			if (m_DisplayModel->IsPackageOK())
			{
				m_DisplayModel->LoadInfo();
			}
			m_DisplayModel->GetViewTLW()->Raise();
			m_DisplayModel->GetView()->Enable();
			m_DisplayModel->GetView()->SetFocus();
		});
	}
}