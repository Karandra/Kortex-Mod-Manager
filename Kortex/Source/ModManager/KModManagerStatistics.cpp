#include "stdafx.h"
#include "KModManagerStatistics.h"
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

void KModManagerStatisticsModel::OnInitControl()
{
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 250);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
}

void KModManagerStatisticsModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	KMMStatisticsEnum index = GetIndex(row);
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
bool KModManagerStatisticsModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	return false;
}

wxString KModManagerStatisticsModel::GetStatName(KMMStatisticsEnum index) const
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
const wxString& KModManagerStatisticsModel::GetStatValue(KMMStatisticsEnum index) const
{
	return m_DataVector[index - 1];
}
wxString KModManagerStatisticsModel::CalcStatValue(KMMStatisticsEnum index) const
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

int64_t KModManagerStatisticsModel::CountMods(CountMode mode) const
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
int64_t KModManagerStatisticsModel::CalcModStoreSize() const
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

void KModManagerStatisticsModel::RefreshItems()
{
	m_DataVector.clear();

	for (int i = KMM_STAT_MIN + 1; i < KMM_STAT_MAX; i++)
	{
		KMMStatisticsEnum index = (KMMStatisticsEnum)i;
		m_DataVector[i - 1] = CalcStatValue(index);
	}
	KDataViewVectorListModel::RefreshItems();
}

//////////////////////////////////////////////////////////////////////////
KModManagerStatisticsDialog::KModManagerStatisticsDialog(wxWindow* parent)
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
		KModManagerStatisticsModel::Create(m_ViewPane, sizer);
		RefreshItems();
		dialog.Hide();

		AdjustWindow(wxDefaultPosition, wxSize(500, 350));
		GetView()->SetFocus();
	}
}
KModManagerStatisticsDialog::~KModManagerStatisticsDialog()
{
	IncRef();
}
