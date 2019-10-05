#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "ImageViewerDialog.h"
#include "Utility/KAux.h"
#include "Utility/UI.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxString.h>

namespace Kortex::Application::OName
{
	KortexDefOption(ImageViewer);

	KortexDefOption(ColorBG);
	KortexDefOption(ColorFG);
}

namespace Kortex::UI
{
	bool ImageViewerEvent::HasBitmap() const
	{
		return GetType() == BitmapPtr;
	}
	wxBitmap ImageViewerEvent::GetBitmap() const
	{
		return HasBitmap() ? std::get<BitmapPtr>(m_Data) : wxNullBitmap;
	}
	void ImageViewerEvent::SetBitmap(const wxBitmap& bitmap)
	{
		m_Data = bitmap.IsOk() ? bitmap : wxNullBitmap;
	}
	
	bool ImageViewerEvent::IsAnimationFile() const
	{
		return HasFilePath() && Kortex::IScreenshotsGallery::IsAnimationFile(GetFilePath());
	}
	bool ImageViewerEvent::HasFilePath() const
	{
		return GetType() == FilePath;
	}
	wxString ImageViewerEvent::GetFilePath() const
	{
		return HasFilePath() ? std::get<FilePath>(m_Data) : KxNullWxString;
	}
	void ImageViewerEvent::SetFilePath(const wxString& filePath)
	{
		m_Data = filePath;
	}
	
	bool ImageViewerEvent::IsInputStream() const
	{
		return GetType() == InputStream;
	}
	wxMemoryInputStream ImageViewerEvent::GetInputSteram()
	{
		if (IsInputStream())
		{
			const KArchive::Buffer* buffer = std::get<InputStream>(m_Data);
			if (buffer)
			{
				return wxMemoryInputStream(buffer->data(), buffer->size());
			}
		}
		return wxMemoryInputStream(nullptr, 0);
	}
	void ImageViewerEvent::SetInputStream(const KArchive::Buffer& buffer)
	{
		m_Data = &buffer;
	}
}

namespace Kortex::UI
{
	bool ImageViewerDialog::OnDynamicBind(wxDynamicEventTableEntry& entry)
	{
		if (entry.m_eventType == ImageViewerEvent::EvtNext)
		{
			m_Forward->SetEnabled(true);
		}
		if (entry.m_eventType == ImageViewerEvent::EvtPrevious)
		{
			m_Backward->SetEnabled(true);
		}
		return KxStdDialog::OnDynamicBind(entry);
	}
	
	void ImageViewerDialog::OnLoadFromDisk(const wxString& filePath)
	{
		if (Kortex::IScreenshotsGallery::IsAnimationFile(filePath))
		{
			m_ImageView->LoadFile(filePath);
		}
		else
		{
			m_ImageView->SetBitmap(wxBitmap(filePath, wxBITMAP_TYPE_ANY));
		}
	}
	void ImageViewerDialog::OnNavigation(wxAuiToolBarEvent& event)
	{
		ImageViewerEvent evt;
		evt.SetEventObject(this);
		if (event.GetEventObject() == m_Backward)
		{
			evt.SetEventType(ImageViewerEvent::EvtPrevious);
		}
		else
		{
			evt.SetEventType(ImageViewerEvent::EvtNext);
		}
		HandleWindowEvent(evt);
		OnAcceptNavigation(evt);
	}
	void ImageViewerDialog::OnAcceptNavigation(ImageViewerEvent& event)
	{
		if (event.IsAllowed())
		{
			m_Backward->SetEnabled(event.HasPrev());
			m_Forward->SetEnabled(event.HasNext());
	
			wxString description = event.GetDescription();
			if (!description.IsEmpty())
			{
				m_Splitter->Unsplit(m_ImageView);
				m_Splitter->Unsplit(m_Description);
	
				m_Description->SetValue(wxString::Format(wxS("<div align=\"center\">%s</div>"), description));
				m_Description->Enable(true);
	
				int nMinHeight = m_Splitter->GetMinimumPaneSize();
				int height = -std::min(m_Description->GetTextExtent(description).GetHeight() + nMinHeight, nMinHeight);
				m_Splitter->SplitHorizontally(m_ImageView, m_Description, height);
			}
			else
			{
				m_Splitter->Unsplit(m_ImageView);
				m_Splitter->Unsplit(m_Description);
				m_Splitter->Initialize(m_ImageView);
	
				m_Description->SetValue(KAux::MakeHTMLWindowPlaceholder(KTr("InstallWizard.NoDescriptionHint"), m_Description));
				m_Description->Enable(false);
			}
	
			if (event.HasBitmap())
			{
				m_ImageView->SetBitmap(event.GetBitmap());
				m_FilePath.Clear();
			}
			else
			{
				m_FilePath = event.GetFilePath();
				OnLoadFromDisk(m_FilePath);
			}
		}
		else
		{
			wxBell();
		}
	}
	void ImageViewerDialog::OnScaleChanged(wxCommandEvent& event)
	{
		m_ImageView->SetScaleFactor(event.GetInt() / 100.0);
	}
	void ImageViewerDialog::OnSaveImage(wxCommandEvent& event)
	{
		KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
		const KxStringVector& exts = Kortex::IScreenshotsGallery::GetSupportedExtensions();
		const Kortex::ScreenshotsGallery::SupportedTypesVector& formats = Kortex::IScreenshotsGallery::GetSupportedFormats();
		for (const wxString& ext: exts)
		{
			dialog.AddFilter(ext, wxString::Format("%s %s", KTr("FileFilter.Image"), ext.AfterFirst('.').MakeUpper()));
		}
		dialog.SetDefaultExtension(exts[0]);
		dialog.SetFileName(m_FilePath.AfterLast('\\').BeforeLast('.'));
	
		if (dialog.ShowModal() == KxID_OK)
		{
			m_ImageView->GetBitmap().SaveFile(dialog.GetResult(), formats[dialog.GetSelectedFilter()]);
		}
	}
	void ImageViewerDialog::OnChangeColor(wxColourPickerEvent& event)
	{
		wxColour color = event.GetColour();
		if (event.GetEventObject() == m_ColorBGCtrl)
		{
			m_ImageView->SetBackgroundColour(color);
		}
		else
		{
			m_ImageView->SetForegroundColour(color);
		}
		m_ImageView->Refresh();
	}
	
	ImageViewerDialog::ImageViewerDialog(wxWindow* parent, const wxString& caption)
	{
		if (Create(parent, caption))
		{
			SetSize(IMainWindow::GetDialogBestSize(this));
			GetButton(KxID_CLOSE).GetControl()->SetFocus();
			CenterOnScreen();
		}
	}
	ImageViewerDialog::~ImageViewerDialog()
	{
		using namespace Application;

		auto options = IApplication::GetInstance()->GetGlobalOption(OName::ImageViewer);
		options.SetAttribute(OName::ColorBG, (int64_t)m_ImageView->GetBackgroundColour().GetPixel());
		options.SetAttribute(OName::ColorFG, (int64_t)m_ImageView->GetForegroundColour().GetPixel());
	}
	
	bool ImageViewerDialog::Create(wxWindow* parent, const wxString& caption)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KAux::StrOr(caption, KTr("ImageViewer.Caption")), wxDefaultPosition, wxDefaultSize, KxBTN_CLOSE))
		{
			AddButton(KxID_SAVE, wxEmptyString, true).GetControl()->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnSaveImage, this);
	
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			SetInitialSize(parent->GetSize().Scale(0.7f, 0.7f));
	
			// Splitter
			m_Splitter = new KxSplitterWindow(m_ContentPanel, KxID_NONE);
			IThemeManager::GetActive().ProcessWindow(m_Splitter);
			PostCreate(wxDefaultPosition);
	
			// View
			m_ImageView = new KxImageView(m_Splitter, KxID_NONE);
			m_ImageView->SetScaleMode(KxIV_SCALE_ASPECT_FIT);
			m_ImageView->SetBackgroundMode(KxIV_BG_TRANSPARENCY_PATTERN);
			m_ImageView->Bind(wxEVT_RIGHT_UP, [this](wxMouseEvent& event)
			{
				Close();
			});
			IThemeManager::GetActive().ProcessWindow(m_ImageView);
	
			// Options
			using namespace Application;

			auto options = IApplication::GetInstance()->GetGlobalOption(OName::ImageViewer);
			int64_t colorBG = options.GetAttributeInt(OName::ColorBG, -1);
			if (colorBG != -1)
			{
				m_ImageView->SetBackgroundColour(KxColor::FromCOLORREF(colorBG));
			}
			else
			{
				m_ImageView->SetBackgroundColour("LIGHT GREY");
			}
	
			int64_t colorFG = options.GetAttributeInt(OName::ColorFG, -1);
			if (colorFG != -1)
			{
				m_ImageView->SetForegroundColour(KxColor::FromCOLORREF(colorFG));
			}
			else
			{
				m_ImageView->SetForegroundColour("WHITE");
			}
			
			// Description
			m_Description = new KxHTMLWindow(m_Splitter, KxID_NONE);
			AddUserWindow(m_Description);
	
			// Split
			m_Splitter->SetSashGravity(0);
			m_Splitter->SetMinimumPaneSize(m_Description->GetCharHeight() * 3);
	
			// Toolbar
			m_ToolBar = new KxAuiToolBar(GetContentWindow(), KxID_NONE, KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND);
			m_ToolBar->AddStretchSpacer(1);
	
			// BG color
			m_ColorBGCtrl = new wxColourPickerCtrl(m_ToolBar, KxID_NONE, m_ImageView->GetBackgroundColour());
			m_ColorBGCtrl->Bind(wxEVT_COLOURPICKER_CHANGED, &ImageViewerDialog::OnChangeColor, this);
			m_ToolBar->AddControl(m_ColorBGCtrl);
	
			// Backward
			m_Backward = Utility::UI::CreateToolBarButton(m_ToolBar, KTr(KxID_BACKWARD), ImageResourceID::ControlLeft);
			
			// Scale
			m_ScaleSlider = new KxSlider(m_ToolBar, KxID_NONE, 100, 10, 500);
			m_ScaleSlider->Bind(wxEVT_SLIDER, &ImageViewerDialog::OnScaleChanged, this);
			m_ToolBar->AddControl(m_ScaleSlider);
	
			// Forward
			m_Forward = Utility::UI::CreateToolBarButton(m_ToolBar, KTr(KxID_FORWARD), ImageResourceID::ControlRight);
			
			// FG color
			m_ColorFGCtrl = new wxColourPickerCtrl(m_ToolBar, KxID_NONE, m_ImageView->GetForegroundColour());
			m_ColorFGCtrl->Bind(wxEVT_COLOURPICKER_CHANGED, &ImageViewerDialog::OnChangeColor, this);
			m_ToolBar->AddControl(m_ColorFGCtrl);
			
			m_ToolBar->AddStretchSpacer(1);
	
			// Realize
			m_Backward->SetEnabled(false);
			m_Forward->SetEnabled(false);
	
			m_Backward->Bind(KxEVT_AUI_TOOLBAR_CLICK, &ImageViewerDialog::OnNavigation, this);
			m_Forward->Bind(KxEVT_AUI_TOOLBAR_CLICK, &ImageViewerDialog::OnNavigation, this);
			
			m_ToolBar->Realize();
	
			m_ToolBar->SetBackgroundColour(GetContentWindow()->GetBackgroundColour());
			GetContentWindowSizer()->Add(m_ToolBar, 0, wxEXPAND);
			AddUserWindow(m_ToolBar);
			return true;
		}
		return false;
	}
}
