#include "stdafx.h"
#include "KProgramOptions.h"
#include "UI/KWorkspace.h"
#include "InstallWizard/KInstallWizardDialog.h"
#include "Profile/KProfile.h"
#include "KManager.h"
#include "KApp.h"
#include <KxFramework/KxDataView.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>
#include <KxFramework/KxSplitterWindow.h>

bool KProgramOptionSerializer::DoCheckSaveLoad(KProgramOption& option, bool save)
{
	if (save)
	{
		return option.IsLoaded();
	}
	else
	{
		option.MarkLoaded();
		return true;
	}
}
void KProgramOptionSerializer::DoSaveLoadDataViewLayout(KxDataViewCtrl* viewCtrl, KProgramOption& option, bool save)
{
	if (!DoCheckSaveLoad(option, save))
	{
		return;
	}

	wxWindowUpdateLocker lock1(viewCtrl);
	wxWindowUpdateLocker lock2(viewCtrl->GetMainWindow());

	wxArrayInt indexes;
	if (!save)
	{
		indexes.resize(viewCtrl->GetColumnCount());
	}

	for (size_t i = 0; i < viewCtrl->GetColumnCount(); i++)
	{
		KxDataViewColumn* column = viewCtrl->GetColumn(i);
		int columnID = column->GetID();

		wxString widthName = wxString::Format("Column[%d].Width", columnID);
		wxString visibleName = wxString::Format("Column[%d].Visible", columnID);
		wxString displayIndexName = wxString::Format("Column[%d].DisplayIndex", columnID);

		if (save)
		{
			if (column->IsResizeable())
			{
				option.SetAttribute(widthName, column->GetWidth());
			}
			option.SetAttribute(visibleName, column->IsShown());
			option.SetAttribute(displayIndexName, (int64_t)viewCtrl->GetColumnPosition(column));
		}
		else
		{
			if (column->IsResizeable())
			{
				column->SetWidth(option.GetAttributeInt(widthName, std::max(wxDVC_DEFAULT_WIDTH, column->GetMinWidth())));
			}
			column->SetHidden(!option.GetAttributeBool(visibleName, true));
			indexes[i] = option.GetAttributeInt(displayIndexName, columnID);
		}
	}

	if (!save)
	{
		if (wxHeaderCtrl* header = viewCtrl->GetHeaderCtrl())
		{
			header->SetColumnsOrder(indexes);
		}

		viewCtrl->UpdateWindowUI();
		viewCtrl->SendSizeEvent();
		viewCtrl->Update();
	}
}
void KProgramOptionSerializer::DoSaveLoadSplitterLayout(KxSplitterWindow* window, KProgramOption& option, bool save)
{
	if (!DoCheckSaveLoad(option, save))
	{
		return;
	}

	if (save)
	{
		option.SetAttribute(window->GetName(), window->GetSashPosition());
	}
	else
	{
		window->SetInitialPosition(option.GetAttributeInt(window->GetName(), window->GetMinimumPaneSize()));
	}
}
void KProgramOptionSerializer::DoSaveLoadWindowSize(wxTopLevelWindow* window, KProgramOption& option, bool save)
{
	if (!DoCheckSaveLoad(option, save))
	{
		return;
	}

	bool maximized = false;
	bool topLevel = false;
	if (window->IsTopLevel())
	{
		topLevel = true;
		maximized = window->IsMaximized();
	}

	if (save)
	{
		if (topLevel)
		{
			option.SetAttribute("Maximized", window->IsMaximized());
		}
		else
		{
			wxSize tSize = window->GetSize();
			option.SetAttribute("Width", tSize.GetWidth());
			option.SetAttribute("Height", tSize.GetHeight());
		}
	}
	else
	{
		if (topLevel)
		{
			if (option.GetAttributeBool("Maximized"))
			{
				window->Maximize();
			}
			else
			{
				window->Center();
			}
		}
		else
		{
			wxSize tSize(option.GetAttributeInt("Width", wxDefaultCoord), option.GetAttributeInt("Height", wxDefaultCoord));
			tSize.DecToIfSpecified(window->GetMaxSize());
			tSize.IncTo(window->GetMinSize());
			window->SetSize(tSize);
		}
	}
}

wxString KProgramOptionSerializer::GetValue(KPGCFileID id, const wxString& section, const wxString& name, const wxString& default)
{
	wxString value = KApp::Get().GetSettingsValue(id, section, name);
	return !value.IsEmpty() ? value : default;
}
void KProgramOptionSerializer::SetValue(KPGCFileID id, const wxString& section, const wxString& name, const wxString& value)
{
	KApp::Get().SetSettingsValue(id, section, name, value);
}

//////////////////////////////////////////////////////////////////////////
bool KProgramOption::DoSetValue(const wxString& value, bool isCDATA)
{
	if (IsPathsOK())
	{
		KProgramOptionSerializer::SetValue(m_FileID, m_SectionID, m_NameID, value);
		return true;
	}
	return false;
}
bool KProgramOption::DoSetAttribute(const wxString& name, const wxString& value)
{
	if (IsPathsOK() && !name.IsEmpty())
	{
		if (!m_NameID.IsEmpty())
		{
			KProgramOptionSerializer::SetValue(m_FileID, m_SectionID + "::" + m_NameID, name, value);
		}
		else
		{
			KProgramOptionSerializer::SetValue(m_FileID, m_SectionID, name, value);
		}
		return true;
	}
	return false;
}

KxIntVector KProgramOption::DoToIntVector(const wxString& values) const
{
	KxIntVector outList;
	KxStringVector list = KxString::Split(values, ";", false);
	if (!list.empty())
	{
		for (const auto& v: list)
		{
			long value = 0;
			if (v.ToCLong(&value))
			{
				outList.push_back(value);
			}
		}
	}
	return outList;
}
wxString KProgramOption::DoFromIntVector(const KxIntVector& data) const
{
	wxString result;
	for (size_t i = 0; i < data.size(); i++)
	{
		result.Append(wxString::Format("%d", data[i]));
		if (i + 1 != data.size())
		{
			result.Append(';');
		}
	}
	return result;
}

KProgramOption::KProgramOption(KWorkspace* workspace, KPGCFileID id, const wxString& nameID)
	:KProgramOption(id, workspace->GetID(), nameID)
{
}
void KProgramOption::Init(KWorkspace* workspace, const wxString& nameID)
{
	SetSection(workspace->GetID());
	if (!nameID.IsEmpty())
	{
		SetName(nameID);
	}
}

/* Value */
wxString KProgramOption::GetValue(const wxString& default) const
{
	return KProgramOptionSerializer::GetValue(m_FileID, m_SectionID, m_NameID, default);
}

/* Attributes */
wxString KProgramOption::GetAttribute(const wxString& name, const wxString& default) const
{
	if (!m_NameID.IsEmpty())
	{
		return KProgramOptionSerializer::GetValue(m_FileID, m_SectionID + "::" + m_NameID, name, default);
	}
	else
	{
		return KProgramOptionSerializer::GetValue(m_FileID, m_SectionID, name, default);
	}
}

//////////////////////////////////////////////////////////////////////////
KProgramOptionUI::KProgramOptionUI(KInstallWizardDialog* workspace, const wxString& nameID)
	:KProgramOption(KPGC_ID_CURRENT_PROFILE, "KInstallWizardDialog", nameID)
{
}
KProgramOptionUI::KProgramOptionUI(KManager* manager, const wxString& nameID)
	:KProgramOption(KPGC_ID_CURRENT_PROFILE, manager->GetID(), nameID)
{
}
