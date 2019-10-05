#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "Workspace.h"
#include "GameInstance/IGameInstance.h"
#include "UI/KImageViewerDialog.h"
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxThumbView.h>
#include <KxFramework/KxShellMenu.h>
#include <KxFramework/KxFile.h>

namespace Kortex::ScreenshotsGallery
{
	bool Workspace::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		m_ViewPane = new KxThumbView(this, KxID_NONE);
		m_ViewPane->SetSpacing(wxSize(KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING));
		m_MainSizer->Add(m_ViewPane, 1, wxEXPAND);
		IThemeManager::GetActive().ProcessWindow(m_ViewPane);

		m_ViewPane->Bind(KxEVT_THUMBVIEW_SELECTED, &Workspace::OnSelectItem, this);
		m_ViewPane->Bind(KxEVT_THUMBVIEW_ACTIVATED, &Workspace::OnActivateItem, this);
		m_ViewPane->Bind(KxEVT_THUMBVIEW_CONTEXT_MENU, &Workspace::OnItemMenu, this);

		OnReloadWorkspace();
		return true;
	}
	bool Workspace::OnOpenWorkspace()
	{
		m_ViewPane->SetFocus();
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		LoadData();
	}

	Workspace::~Workspace()
	{
	}

	void Workspace::LoadData()
	{
		m_ViewPane->ClearThumbs();
		m_LoadedImages.clear();

		IScreenshotsGallery* manager = IScreenshotsGallery::GetInstance();
		for (const wxString& folderPath: manager->GetConfig().GetLocations())
		{
			KxStringVector files = KxFile(KVarExp(folderPath)).Find(manager->GetSupportedExtensions(), KxFS_FILE, false);
			for (const wxString& path: files)
			{
				m_LoadedImages.emplace_back(path);
				m_ViewPane->AddThumb(path);
			}
		}
	}
	void Workspace::OnSelectItem(wxCommandEvent& event)
	{
		if (event.GetInt() != wxNOT_FOUND)
		{
			DisplayInfo(m_LoadedImages[event.GetInt()]);
		}
		else
		{
			ClearControls();
		}
	}
	void Workspace::OnActivateItem(wxCommandEvent& event)
	{
		if (event.GetInt() != wxNOT_FOUND)
		{
			m_CurrentImageIndex = event.GetInt();
			UI::KImageViewerDialog dialog(this);
			dialog.Bind(UI::KImageViewerEvent::EvtPrevious, &Workspace::OnDialogNavigate, this);
			dialog.Bind(UI::KImageViewerEvent::EvtNext, &Workspace::OnDialogNavigate, this);

			UI::KImageViewerEvent evt;
			SetNavigationInfo(evt);
			evt.SetFilePath(m_LoadedImages[m_CurrentImageIndex]);
			dialog.Navigate(evt);

			dialog.ShowModal();
			m_CurrentImageIndex = -1;
		}
	}
	void Workspace::OnItemMenu(wxContextMenuEvent& event)
	{
		if (event.GetInt() != wxNOT_FOUND)
		{
			const wxString path = m_LoadedImages[event.GetInt()];
			KxShellMenu menu(path);
			if (menu.IsOK())
			{
				menu.Bind(KxEVT_MENU_HOVER, [](KxMenuEvent& event)
				{
					IMainWindow::GetInstance()->SetStatus(event.GetHelpString());
				});
				menu.Show(this, event.GetPosition());
			}
		}
	}
	void Workspace::OnDialogNavigate(UI::KImageViewerEvent& event)
	{
		int oldIndex = m_CurrentImageIndex;
		if (event.GetEventType() == UI::KImageViewerEvent::EvtNext)
		{
			m_CurrentImageIndex++;
		}
		else
		{
			m_CurrentImageIndex--;
		}

		SetNavigationInfo(event);
		if (m_CurrentImageIndex >= 0 && (size_t)m_CurrentImageIndex < m_LoadedImages.size())
		{
			event.SetFilePath(m_LoadedImages[m_CurrentImageIndex]);
		}
		else
		{
			m_CurrentImageIndex = oldIndex;
			event.Veto();
		}
	}
	void Workspace::SetNavigationInfo(UI::KImageViewerEvent& event)
	{
		event.SetHasPrevNext(m_CurrentImageIndex > 0, (size_t)(m_CurrentImageIndex + 1) < m_LoadedImages.size());
	}

	wxString Workspace::GetID() const
	{
		return "KScreenshotsGalleryWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("ScreenshotsGallery.Name");
	}

	void Workspace::DisplayInfo(const wxString& filePath)
	{
		ClearControls();
	}
	void Workspace::ClearControls()
	{
	}
}
