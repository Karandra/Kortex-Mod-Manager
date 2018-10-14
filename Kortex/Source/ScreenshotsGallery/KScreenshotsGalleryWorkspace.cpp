#include "stdafx.h"
#include "KScreenshotsGalleryManager.h"
#include "KScreenshotsGalleryWorkspace.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "GameInstance/KGameInstance.h"
#include "GameInstance/Config/KScreenshotsGalleryConfig.h"
#include "UI/KImageViewerDialog.h"
#include <KxFramework/KxParagraph.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxThumbView.h>
#include <KxFramework/KxShellMenu.h>
#include <KxFramework/KxFile.h>

KScreenshotsGalleryWorkspace::KScreenshotsGalleryWorkspace(KMainWindow* mainWindow, KScreenshotsGalleryManager* manager)
	:KWorkspace(mainWindow), m_Manager(manager)
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KScreenshotsGalleryWorkspace::~KScreenshotsGalleryWorkspace()
{
}
bool KScreenshotsGalleryWorkspace::OnCreateWorkspace()
{
	m_ViewPane = new KxThumbView(this, KxID_NONE);
	m_ViewPane->SetSpacing(wxSize(KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING));
	m_MainSizer->Add(m_ViewPane, 1, wxEXPAND);
	KThemeManager::Get().ProcessWindow(m_ViewPane);

	m_ViewPane->Bind(KxEVT_THUMBVIEW_SELECTED, &KScreenshotsGalleryWorkspace::OnSelectItem, this);
	m_ViewPane->Bind(KxEVT_THUMBVIEW_ACTIVATED, &KScreenshotsGalleryWorkspace::OnActivateItem, this);
	m_ViewPane->Bind(KxEVT_THUMBVIEW_CONTEXT_MENU, &KScreenshotsGalleryWorkspace::OnItemMenu, this);

	OnReloadWorkspace();
	return true;
}

void KScreenshotsGalleryWorkspace::LoadData()
{
	m_ViewPane->ClearThumbs();
	m_LoadedImages.clear();

	const KScreenshotsGalleryConfig* galleryConfig = KScreenshotsGalleryConfig::GetInstance();
	if (galleryConfig)
	{
		for (const wxString& folderPath: galleryConfig->GetLocations())
		{
			KxStringVector files = KxFile(V(folderPath)).Find(m_Manager->GetSupportedExtensions(), KxFS_FILE, false);
			for (const wxString& path: files)
			{
				m_LoadedImages.emplace_back(path);
				m_ViewPane->AddThumb(path);
			}
		}
	}
}
void KScreenshotsGalleryWorkspace::OnSelectItem(wxCommandEvent& event)
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
void KScreenshotsGalleryWorkspace::OnActivateItem(wxCommandEvent& event)
{
	if (event.GetInt() != wxNOT_FOUND)
	{
		m_CurrentImageIndex = event.GetInt();
		KImageViewerDialog dialog(this);
		dialog.Bind(KEVT_IMAGEVIEWER_PREV_IMAGE, &KScreenshotsGalleryWorkspace::OnDialogNavigate, this);
		dialog.Bind(KEVT_IMAGEVIEWER_NEXT_IMAGE, &KScreenshotsGalleryWorkspace::OnDialogNavigate, this);

		KImageViewerEvent evt;
		SetNavigationInfo(evt);
		evt.SetFilePath(m_LoadedImages[m_CurrentImageIndex]);
		dialog.Navigate(evt);

		dialog.ShowModal();
		m_CurrentImageIndex = -1;
	}
}
void KScreenshotsGalleryWorkspace::OnItemMenu(wxContextMenuEvent& event)
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
void KScreenshotsGalleryWorkspace::OnDialogNavigate(KImageViewerEvent& event)
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
void KScreenshotsGalleryWorkspace::SetNavigationInfo(KImageViewerEvent& event)
{
	event.SetHasPrevNext(m_CurrentImageIndex > 0, (size_t)(m_CurrentImageIndex + 1) < m_LoadedImages.size());
}

bool KScreenshotsGalleryWorkspace::OnOpenWorkspace()
{
	m_ViewPane->SetFocus();
	return true;
}
bool KScreenshotsGalleryWorkspace::OnCloseWorkspace()
{
	return true;
}
void KScreenshotsGalleryWorkspace::OnReloadWorkspace()
{
	LoadData();
}

wxString KScreenshotsGalleryWorkspace::GetID() const
{
	return "KScreenshotsGalleryWorkspace";
}
wxString KScreenshotsGalleryWorkspace::GetName() const
{
	return T("ScreenshotsGallery.Name");
}

void KScreenshotsGalleryWorkspace::DisplayInfo(const wxString& filePath)
{
	ClearControls();
}
void KScreenshotsGalleryWorkspace::ClearControls()
{
}
