#include "stdafx.h"
#include "KModStatistics.h"
#include "KModManager.h"
#include "KAux.h"
#include "KApp.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProgressDialog.h>

enum ColumnID
{
	Name,
	Value
};

void KModStatisticsModel::OnInitControl()
{
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 250);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
}

void KModStatisticsModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	KModStatisticsType index = GetIndex(row);
	if (index != KMM_STAT_INVALID)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				data = GetStatName(index);
				break;
			}
			case ColumnID::Value:
			{
				data = GetStatValue(index);
				break;
			}
		};
	}
}
bool KModStatisticsModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	return false;
}

wxString KModStatisticsModel::GetStatName(KModStatisticsType index) const
{
	switch (index)
	{
		case KMM_STAT_MOD_COUNT_TOTAL:
		{
			return T("ModManager.Statistics.ModCountTotal");
		}
		case KMM_STAT_MOD_COUNT_ACTIVE:
		{
			return T("ModManager.Statistics.ModCountActive");
		}
		case KMM_STAT_MOD_COUNT_INACTIVE:
		{
			return T("ModManager.Statistics.ModCountInactive");
		}
		case KMM_STAT_MOD_COUNT_FILES:
		{
			return T("ModManager.Statistics.FilesCount");
		}
		case KMM_STAT_MOD_COUNT_FOLDERS:
		{
			return T("ModManager.Statistics.FoldersCount");
		}
		case KMM_STAT_MODS_SIZE:
		{
			return T("ModManager.Statistics.ModsSize");
		}
	};
	return wxEmptyString;
}
const wxString& KModStatisticsModel::GetStatValue(KModStatisticsType index) const
{
	return m_DataVector[index - 1];
}
wxString KModStatisticsModel::CalcStatValue(KModStatisticsType index) const
{
	switch (index)
	{
		case KMM_STAT_MOD_COUNT_TOTAL:
		{
			return std::to_wstring(CountMods(All));
		}
		case KMM_STAT_MOD_COUNT_ACTIVE:
		{
			return std::to_wstring(CountMods(Active));
		}
		case KMM_STAT_MOD_COUNT_INACTIVE:
		{
			return std::to_wstring(CountMods(Inactive));
		}
		case KMM_STAT_MOD_COUNT_FILES:
		{
			return std::to_wstring(CountFilesAndFolders(KxFS_FILE, All));
		}
		case KMM_STAT_MOD_COUNT_FOLDERS:
		{
			return std::to_wstring(CountFilesAndFolders(KxFS_FOLDER, All));
		}
		case KMM_STAT_MODS_SIZE:
		{
			return KxFile::FormatFileSize(CalcModStoreSize(), 2);
		}
	};
	return wxEmptyString;
}

int64_t KModStatisticsModel::CountMods(CountMode mode) const
{
	int64_t count = 0;
	for (const KModEntry* modEentry: KModManager::Get().GetAllEntries(true))
	{
		switch (mode)
		{
			case Active:
			{
				if (modEentry->IsEnabled())
				{
					count++;
				}
				break;
			}
			case Inactive:
			{
				if (!modEentry->IsEnabled())
				{
					count++;
				}
				break;
			}
			default:
			{
				count++;
				break;
			}
		};
	}
	return count;
}
int64_t KModStatisticsModel::CalcModStoreSize() const
{
	int64_t totalSize = 0;
	for (const KModEntry* modEntry: KModManager::Get().GetAllEntries(true))
	{
		modEntry->GetFileTree().WalkTree([&totalSize](const KFileTreeNode& rootNode)
		{
			for (const KFileTreeNode& node: rootNode.GetChildren())
			{
				if (node.IsFile())
				{
					totalSize += node.GetFileSize();
				}
			}
			return true;
		});
	}
	return totalSize;
}
int64_t KModStatisticsModel::CountFilesAndFolders(KxFileSearchType type, CountMode mode) const
{
	int64_t count = 0;
	for (const KModEntry* modEntry: KModManager::Get().GetAllEntries(true))
	{
		modEntry->GetFileTree().WalkTree([&count, type, mode](const KFileTreeNode& rootNode)
		{
			for (const KFileTreeNode& node: rootNode.GetChildren())
			{
				if (node.GetItem().IsElementType(type))
				{
					switch (mode)
					{
						case Active:
						{
							if (rootNode.GetMod().IsEnabled())
							{
								count++;
							}
							break;
						}
						case Inactive:
						{
							if (!rootNode.GetMod().IsEnabled())
							{
								count++;
							}
							break;
						}
						default:
						{
							count++;
							break;
						}
					};
				}
			}
			return true;
		});
	}
	return count;
}

void KModStatisticsModel::RefreshItems()
{
	m_DataVector.clear();

	for (int i = KMM_STAT_MIN + 1; i < KMM_STAT_MAX; i++)
	{
		KModStatisticsType index = (KModStatisticsType)i;
		m_DataVector[i - 1] = CalcStatValue(index);
	}
	KDataViewVectorListModel::RefreshItems();
}

//////////////////////////////////////////////////////////////////////////
KModStatisticsDialog::KModStatisticsDialog(wxWindow* parent)
{
	if (KxStdDialog::Create(parent, KxID_NONE, T("ModManager.Statistics"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		KxProgressDialog dialog(parent, KxID_NONE, T("ModManager.Statistics.Status"), wxDefaultPosition, wxDefaultSize, KxBTN_NONE);
		dialog.Pulse();
		dialog.Show();
		KApp::Get().Yield();

		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		// List
		KModStatisticsModel::Create(m_ViewPane, sizer);
		RefreshItems();
		dialog.Hide();

		AdjustWindow(wxDefaultPosition, wxSize(500, 350));
		GetView()->SetFocus();
	}
}
KModStatisticsDialog::~KModStatisticsDialog()
{
	IncRef();
}
