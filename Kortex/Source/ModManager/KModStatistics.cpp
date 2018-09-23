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
		case KMM_STAT_MOD_COUNT_ALL:
		{
			return T("ModManager.Statistics.ModCountAll");
		}
		case KMM_STAT_MOD_COUNT_ACTIVE:
		{
			return T("ModManager.Statistics.ModCountActive");
		}
		case KMM_STAT_MOD_COUNT_INACTIVE:
		{
			return T("ModManager.Statistics.ModCountInactive");
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
		case KMM_STAT_MOD_COUNT_ALL:
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
	for (const KModEntry* entry: KModManager::Get().GetEntries())
	{
		switch (mode)
		{
			case Active:
			{
				if (entry->IsEnabled())
				{
					count++;
				}
				break;
			}
			case Inactive:
			{
				if (!entry->IsEnabled())
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
	std::function<void(const KFileTreeNode&)> ScanTree = [&ScanTree, &totalSize](const KFileTreeNode& rootNode)
	{
		for (const KFileTreeNode& node: rootNode.GetChildren())
		{
			if (node.IsFile())
			{
				totalSize += node.GetFileSize();
			}
			else
			{
				ScanTree(node);
			}
		}
	};

	// Only real mods, no base game and no overwrite.
	for (const KModEntry* modEntry: KModManager::Get().GetEntries())
	{
		ScanTree(modEntry->GetFileTree());
	}
	return totalSize;
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
