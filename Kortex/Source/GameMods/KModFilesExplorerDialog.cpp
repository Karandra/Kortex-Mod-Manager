#include "stdafx.h"
#include "KModFilesExplorerDialog.h"
#include "KModFilesExplorerModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"

namespace Kortex::ModManager
{
	void KModFilesExplorerDialog::CreateUI(wxWindow* parent)
	{
		wxString caption = wxString::Format("%s \"%s\"", KTr("ModExplorer.Caption"), m_Mod.GetName());
		if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			m_ViewModel = new KModFilesExplorerModel(m_Mod);
			m_ViewModel->Create(m_ViewPane, sizer);
			m_ViewModel->RefreshItems();

			AdjustWindow(wxDefaultPosition, wxSize(720, 500));
			//KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}

	KModFilesExplorerDialog::KModFilesExplorerDialog(wxWindow* parent, const IGameMod& mod)
		:m_Mod(mod)
		//m_ViewOptions("KModFilesExplorerDialog", "FilesView"), m_WindowOptions("KModFilesExplorerDialog", "Window")
	{
		CreateUI(parent);
	}
	KModFilesExplorerDialog::~KModFilesExplorerDialog()
	{
		//KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
