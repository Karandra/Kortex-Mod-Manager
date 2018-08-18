#include "stdafx.h"
#include "KModFilesExplorerDialog.h"
#include "KModFilesExplorerModel.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"

void KModFilesExplorerDialog::CreateUI(wxWindow* parent)
{
	wxString caption = HasModEntry() ? wxString::Format("%s \"%s\"", T("ModExplorer.Caption"), m_ModEntry->GetName()) : T("ModExplorer.Caption");
	if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		m_ViewModel = new KModFilesExplorerModel(m_ExplorerRoot, m_ModEntry);
		m_ViewModel->Create(m_ViewPane, sizer);
		m_ViewModel->RefreshItems();

		AdjustWindow(wxDefaultPosition, wxSize(720, 500));
		KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
		KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
	}
}

KModFilesExplorerDialog::KModFilesExplorerDialog(wxWindow* 
												 parent, const KModEntry& modEntry)
	:m_ModEntry(&modEntry), m_ExplorerRoot(modEntry.GetLocation(KMM_LOCATION_MOD_FILES)),
	m_ViewOptions("KModFilesExplorerDialog", "FilesView"),
	m_WindowOptions("KModFilesExplorerDialog", "Window")
{
	CreateUI(parent);
}
KModFilesExplorerDialog::KModFilesExplorerDialog(wxWindow* parent, const wxString& explorerRoot)
	:m_ModEntry(NULL), m_ExplorerRoot(explorerRoot),
	m_ViewOptions("KModFilesExplorerDialog", "FilesView"),
	m_WindowOptions("KModFilesExplorerDialog", "Window")
{
	CreateUI(parent);
}
KModFilesExplorerDialog::~KModFilesExplorerDialog()
{
	KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}
