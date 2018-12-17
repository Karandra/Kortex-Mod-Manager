#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "Workspace.h"
#include "GameInstance/IGameInstance.h"
#include "UI/KImageViewerDialog.h"
#include <KxFramework/KxParagraph.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxThumbView.h>
#include <KxFramework/KxShellMenu.h>
#include <KxFramework/KxFile.h>

namespace Kortex::ScreenshotsGallery
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_Manager = IScreenshotsGallery::GetInstance();
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
	}
	bool Workspace::OnCreateWorkspace()
	{
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

	void Workspace::LoadData()
	{
		m_ViewPane->ClearThumbs();
		m_LoadedImages.clear();

		for (const wxString& folderPath: m_Manager->GetConfig().GetLocations())
		{
			KxStringVector files = KxFile(KVarExp(folderPath)).Find(m_Manager->GetSupportedExtensions(), KxFS_FILE, false);
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
			KImageViewerDialog dialog(this);
			dialog.Bind(KEVT_IMAGEVIEWER_PREV_IMAGE, &Workspace::OnDialogNavigate, this);
			dialog.Bind(KEVT_IMAGEVIEWER_NEXT_IMAGE, &Workspace::OnDialogNavigate, this);

			KImageViewerEvent evt;
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
				menu.Bind(KxEVT_MENU_HOVER, [this](KxMenuEvent& event)
				{
					GetMainWindow()->SetStatus(event.GetHelpString());
				});
				menu.Show(GetMainWindow(), event.GetPosition());
			}
		}
	}
	void Workspace::OnDialogNavigate(KImageViewerEvent& event)
	{
		int oldIndex = m_CurrentImageIndex;
		if (event.GetEventType() == KEVT_IMAGEVIEWER_NEXT_IMAGE)
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
	void Workspace::SetNavigationInfo(KImageViewerEvent& event)
	{
		event.SetHasPrevNext(m_CurrentImageIndex > 0, (size_t)(m_CurrentImageIndex + 1) < m_LoadedImages.size());
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
