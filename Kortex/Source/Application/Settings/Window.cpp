#include "stdafx.h"
#include "Window.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxButton.h>

namespace Kortex::Application::Settings
{
	void Window::OnPrepareUninstall(wxCommandEvent& event)
	{
		KxTaskDialog askDialog(this, KxID_NONE, KTrf("Settings.PrepareUninstall.Caption", Kortex::IApplication::GetInstance()->GetName()), KTr("Settings.PrepareUninstall.Message"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (askDialog.ShowModal() == KxID_YES)
		{
			if (Kortex::IApplication::GetInstance()->Uninstall())
			{
				KxTaskDialog dialog(this, KxID_NONE, KTr("Settings.PrepareUninstall.Success"), wxEmptyString, KxBTN_NONE, KxICON_INFORMATION);
				dialog.AddButton(KxID_OK, KTr("Settings.PrepareUninstall.RebootNow"));
				dialog.AddButton(KxID_CANCEL, KTr("Settings.PrepareUninstall.RebootLater"));
				if (dialog.ShowModal() == KxID_OK)
				{
					wxShutdown(wxSHUTDOWN_REBOOT);
					KMainWindow::GetInstance()->Close(true);
				}
			}
			else
			{
				KxTaskDialog(this, KxID_NONE, KTr("Settings.PrepareUninstall.Error"), wxEmptyString, KxBTN_OK, KxICON_ERROR).ShowModal();
			}
		}
	}

	Window::Window(wxWindow* parent)
		:m_DisplayModel(m_Manager)
	{
		m_Manager.OnInit();
		if (Create(parent ? parent : KMainWindow::GetInstance(), KxID_NONE, KTr("Settings.Caption"), wxDefaultPosition, wxSize(800, 600), KxBTN_OK|KxBTN_CANCEL))
		{
			wxWindow* removeButton = AddButton(KxID_REMOVE, KTr("Settings.PrepareUninstall.Button"), true).GetControl();
			removeButton->Bind(wxEVT_BUTTON, &Window::OnPrepareUninstall, this);

			SetMainIcon(KxICON_NONE);
			PostCreate();

			IThemeManager::GetActive().ProcessWindow(GetContentWindow());
			m_DisplayModel.CreateView(GetContentWindow(), GetContentWindowMainSizer());
			m_DisplayModel.LoadView();
		}
	}
	Window::~Window()
	{
		m_Manager.OnExit();
	}
}
