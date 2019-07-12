#include "stdafx.h"
#include "TextEditDialog.h"
#include "KMainWindow.h"
#include "Utility/KAux.h"
#include <Kortex/Application.hpp>
#include "Application/Resources/ImageResourceID.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStyledTextBox.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxBitmapComboBox.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxUtility.h>
#include <memory>

namespace Kortex::UI
{
	bool TextEditDialog::Create(wxWindow* parent)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("TextEditor.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			SetInitialSize(parent->GetSize().Scale(0.85f, 0.85f));

			/* View */
			wxBoxSizer* viewSizer = new wxBoxSizer(wxVERTICAL);
			m_View = new KxPanel(m_ContentPanel, KxID_NONE);
			m_View->SetSizer(viewSizer);
			IThemeManager::GetActive().ProcessWindow(m_View);

			/* ToolBar */
			m_ToolBar = new KxAuiToolBar(m_View, KxID_NONE, KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND);
			m_ToolBar->SetBackgroundColour(m_View->GetBackgroundColour());
			viewSizer->Add(m_ToolBar, 0, wxEXPAND);

			// Edit mode
			m_ToolBar_SwitchMode = KMainWindow::CreateToolBarButton(m_ToolBar, KTr("TextEditor.ToolBar.Mode"), ImageResourceID::Edit);
			m_ToolBar_SwitchMode->Bind(KxEVT_AUI_TOOLBAR_CLICK, &TextEditDialog::OnSwitchMode, this);
			m_ToolBar->AddSeparator();

			// Save/Load
			m_ToolBar_Save = KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_SAVE), ImageResourceID::Disk);
			m_ToolBar_Save->Bind(KxEVT_AUI_TOOLBAR_CLICK, &TextEditDialog::OnSaveLoadFile, this);

			m_ToolBar_Open = KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_OPEN), ImageResourceID::FolderOpen);
			m_ToolBar_Open->Bind(KxEVT_AUI_TOOLBAR_CLICK, &TextEditDialog::OnSaveLoadFile, this);
			m_ToolBar->AddSeparator();

			// Undo/Redo
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_UNDO), ImageResourceID::ArrowCurve180Left)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				m_Editor->Undo();
			});
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_REDO), ImageResourceID::ArrowCircle135Left)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				m_Editor->Redo();
			});
			m_ToolBar->AddSeparator();

			// Styles
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_BOLD), ImageResourceID::EditBold)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("b");
			});
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_ITALIC), ImageResourceID::EditItalic)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("i");
			});
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_UNDERLINE), ImageResourceID::EditUnderline)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("u");
			});
			m_ToolBar->AddSeparator();

			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_JUSTIFY_LEFT), ImageResourceID::EditAlignmentLeft)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("div", "align", "left");
			});
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_JUSTIFY_CENTER), ImageResourceID::EditAlignmentCenter)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("div", "align", "center");
			});
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_JUSTIFY_RIGHT), ImageResourceID::EditAlignmentRight)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("div", "align", "right");
			});
			KMainWindow::CreateToolBarButton(m_ToolBar, KTr(KxID_JUSTIFY_FILL), ImageResourceID::EditAlignmentJustify)->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				ToggleTag("div", "align", "justify");
			});

			// Heading
			m_HeadingList = new KxBitmapComboBox(m_ToolBar, KxID_ANY);
			m_HeadingList->SetImageList(const_cast<KxImageList*>(&ImageProvider::GetImageList()));
			m_ToolBar->AddControl(m_HeadingList, KTr("TextEditor.ToolBar.Heading"));

			const int maxHeading = 6;
			for (int i = 1; i <= maxHeading; i++)
			{
				m_HeadingList->AddItem(KxString::Format("%1 %2", KTr("TextEditor.ToolBar.Heading"), i), (int)ImageResourceID::EditHeading + i);
			}
			m_HeadingList->SetSelection(0);
			m_HeadingList->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
			{
				ToggleTag(KxString::Format("h%1", event.GetInt() + 1));
				m_Editor->SetFocus();
			});

			m_ToolBar->Realize();

			// Tabs
			m_Container = new wxSimplebook(m_View, KxID_NONE);
			viewSizer->Add(m_Container, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
			IThemeManager::GetActive().ProcessWindow(m_Container);

			PostCreate(wxDefaultPosition);

			/* Editor page */
			m_Editor = new KxStyledTextBox(m_Container, KxID_NONE);
			m_Container->AddPage(m_Editor, wxEmptyString, true);

			/* Preview page */
			m_Preview.Create(m_Container);
			m_Container->AddPage(m_Preview, wxEmptyString);

			/* Complete creation */
			AddUserWindow(m_ToolBar);
			AddUserWindow(m_Editor);
			AddUserWindow(m_Preview);
			AdjustWindow(wxDefaultPosition);
			m_Editor->SetFocus();
			return true;
		}
		return false;
	}

	void TextEditDialog::OnNewTextSet()
	{
		m_Editor->SetValue(m_Text);
		ClearUndoHistory();
	}
	void TextEditDialog::OnPrepareSaveText()
	{
		m_Text = m_Editor->GetValue();
	}
	void TextEditDialog::ClearUndoHistory()
	{
		m_Editor->SetModified(false);
		m_Editor->EmptyUndoBuffer();
	}
	void TextEditDialog::OnSwitchMode(KxAuiToolBarEvent& event)
	{
		DoShowPreview(m_EditMode);
	}
	void TextEditDialog::OnKey(wxKeyEvent& event)
	{
		if (event.ControlDown() && event.GetKeyCode() == WXK_TAB)
		{
			DoShowPreview(m_EditMode);
		}
		else
		{
			event.Skip();
		}
	}
	void TextEditDialog::OnOK(wxNotifyEvent& event)
	{
		if (event.GetId() == KxID_OK)
		{
			OnPrepareSaveText();
			m_IsTextModified = m_Editor->IsModified();
		}
		else
		{
			m_IsTextModified = false;
		}
		event.Skip();
	}
	void TextEditDialog::OnSaveLoadFile(KxAuiToolBarEvent& event)
	{
		bool save = event.GetEventObject() == m_ToolBar_Save;

		KxFileBrowseDialog dialog(this, KxID_NONE, save ? KxFBD_SAVE : KxFBD_OPEN);
		dialog.AddFilter("*.txt", KTr("FileFilter.Text"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
		dialog.SetDefaultExtension("txt");

		if (dialog.ShowModal() == KxID_OK)
		{
			if (save)
			{
				OnPrepareSaveText();
				SaveToFile(dialog.GetResult());
			}
			else
			{
				LoadFromFile(dialog.GetResult());
			}
		}
	}

	void TextEditDialog::ToggleTag(const wxString& tagStart, const wxString& tagEnd)
	{
		int selStart = m_Editor->GetSelectionStart();
		wxString text = m_Editor->GetSelectedText();
		if (text.StartsWith('<' + tagStart) && (text.EndsWith('/' + tagEnd + '>') || text.EndsWith("/>")))
		{
			wxString innerText = text.AfterFirst('>').BeforeLast('<');
			m_Editor->ReplaceSelection(innerText);
		}
		else
		{
			wxString string = wxString::Format("<%s>%s</%s>", tagEnd, text, tagEnd);
			m_Editor->ReplaceSelection(string);
		}
	}
	void TextEditDialog::LoadFromFile(const wxString& filePath)
	{
		m_Text = KxTextFile::ReadToString(filePath);
		OnNewTextSet();
	}
	void TextEditDialog::SaveToFile(const wxString& filePath) const
	{
		KxTextFile::WriteToFile(filePath, m_Text);
	}
	void TextEditDialog::DoShowPreview(bool show)
	{
		if (show)
		{
			m_Preview.LoadText(m_Editor->GetValue());
			m_Container->ChangeSelection(1);
			m_ToolBar_SwitchMode->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::EditCodeDivision));
			m_Preview.GetWindow()->SetFocus();

			m_EditMode = false;
		}
		else
		{
			m_Container->ChangeSelection(0);
			m_ToolBar_SwitchMode->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Edit));
			m_Editor->SetEditable(IsEditable());
			m_Editor->SetFocus();

			m_EditMode = true;
		}

		for (size_t i = 0; i < m_ToolBar->GetToolCount(); i++)
		{
			KxAuiToolBarItem* item = m_ToolBar->FindToolByIndex(i);
			if (item)
			{
				item->SetEnabled(m_EditMode);
			}
		}
		m_ToolBar_SwitchMode->SetEnabled(true);
		m_ToolBar_Save->SetEnabled(true);
		m_HeadingList->Enable(m_EditMode);
	}

	TextEditDialog::TextEditDialog(wxWindow* parent)
	{
		if (Create(parent))
		{
			SetSize(KMainWindow::GetDialogBestSize(this));
			CenterOnScreen();

			Bind(wxEVT_CHAR_HOOK, &TextEditDialog::OnKey, this);
			Bind(KxEVT_STDDIALOG_BUTTON, &TextEditDialog::OnOK, this);
		}
	}
	TextEditDialog::~TextEditDialog()
	{
	}

	int TextEditDialog::ShowModal()
	{
		DoShowPreview(!m_EditMode);
		return KxStdDialog::ShowModal();
	}

	const wxString& TextEditDialog::GetText() const
	{
		return m_Text;
	}
	void TextEditDialog::SetText(const wxString& text)
	{
		m_Text = text;
		OnNewTextSet();
	}
}
