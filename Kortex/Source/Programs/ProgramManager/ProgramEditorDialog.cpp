#include "stdafx.h"
#include "ProgramEditorDialog.h"
#include <Kortex/ProgramManager.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileItem.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxButton.h>

namespace Kortex::ProgramManager
{
	void ProgramEditorDialog::CreateUI(wxWindow* parent)
	{
		wxString caption = m_Program ? KTr("ProgramManager.Menu.EditProgram") : KTr("ProgramManager.Menu.AddProgram");
		if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, {640, wxDefaultCoord}, KxBTN_OK|KxBTN_CANCEL))
		{
			wxFlexGridSizer* mainSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING);
			mainSizer->AddGrowableCol(1, 1);

			m_ContentPanel = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ContentPanel->SetSizer(mainSizer);

			// Name
			{
				KxLabel* label = new KxLabel(m_ContentPanel, KxID_NONE, KTr("Generic.Name") + wxS(":"));
				m_NameInput = new KxTextBox(m_ContentPanel, KxID_NONE);
				m_NameInput->SetFocus();

				mainSizer->Add(label);
				mainSizer->Add(m_NameInput, 1, wxEXPAND);
			}

			// Executable
			{
				KxLabel* label = new KxLabel(m_ContentPanel, KxID_NONE, KTr("ProgramManager.List.Executable") + wxS(":"));
				m_ExecutableInput = new KxTextBox(m_ContentPanel, KxID_NONE);

				KxButton* button = new KxButton(m_ContentPanel, KxID_NONE, KTr("Generic.BrowseFile"));
				button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
				{
					if (wxString path = BrowseForLocation(m_ExecutableInput->GetValue(), false); !path.IsEmpty())
					{
						m_ExecutableInput->SetValue(path);
						if (m_NameInput->IsEmpty())
						{
							m_NameInput->SetValue(path.AfterLast(wxS('\\')).BeforeLast(wxS('.')));
						}
					}
				});

				mainSizer->Add(label);

				wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
				sizer->Add(m_ExecutableInput, 1, wxEXPAND);
				sizer->Add(button, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
				mainSizer->Add(sizer, 1, wxEXPAND);
			}

			// Working folder
			{
				KxLabel* label = new KxLabel(m_ContentPanel, KxID_NONE, KTr("ProgramManager.List.WorkingDirectory") + wxS(":"));
				m_WorkingFolderInput = new KxTextBox(m_ContentPanel, KxID_NONE);

				KxButton* button = new KxButton(m_ContentPanel, KxID_NONE, KTr("Generic.BrowseFolder"));
				button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
				{
					if (wxString path = BrowseForLocation(m_WorkingFolderInput->GetValue(), true); !path.IsEmpty())
					{
						m_WorkingFolderInput->SetValue(path);
					}
				});

				mainSizer->Add(label);

				wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
				sizer->Add(m_WorkingFolderInput, 1, wxEXPAND);
				sizer->Add(button, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
				mainSizer->Add(sizer, 1, wxEXPAND);
			}

			// Arguments
			{
				KxLabel* label = new KxLabel(m_ContentPanel, KxID_NONE, KTr("ProgramManager.List.Arguments") + wxS(":"));
				m_ArgumentsInput = new KxTextBox(m_ContentPanel, KxID_NONE);
				
				wxFont font = m_ArgumentsInput->GetFont();
				font.SetFaceName(wxS("Consolas"));
				m_ArgumentsInput->SetFont(font);

				mainSizer->Add(label);
				mainSizer->Add(m_ArgumentsInput, 1, wxEXPAND);
			}

			SetupValues();

			SetMainIcon(KxICON_NONE);
			PostCreate();
			AdjustWindow();
			Center();
		}
	}
	void ProgramEditorDialog::SetupValues()
	{
		if (m_Program)
		{
			m_NameInput->SetValue(m_Program->RawGetName());
			m_ExecutableInput->SetValue(m_Program->RawGetExecutable());
			m_WorkingFolderInput->SetValue(m_Program->RawGetWorkingDirectory());
			m_ArgumentsInput->SetValue(m_Program->RawGetArguments());
		}
	}
	wxString ProgramEditorDialog::BrowseForLocation(const wxString& path, bool isDirectory)
	{
		KxFileBrowseDialog dialog(this, KxID_NONE, isDirectory ? KxFBD_OPEN_FOLDER : KxFBD_OPEN);
		if (isDirectory)
		{
			dialog.SetFolder(path.BeforeLast('\\'));
		}
		else
		{
			dialog.SetFolder(path);
			dialog.AddFilter("*.exe", KTr("FileFilter.Programs"));
			dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
		}

		if (dialog.ShowModal())
		{
			return dialog.GetResult();
		}
		return wxEmptyString;
	}

	IProgramEntry& ProgramEditorDialog::Accept()
	{
		auto SetData = [this](IProgramEntry& program)
		{
			program.SetName(m_NameInput->GetValue());
			program.SetExecutable(m_ExecutableInput->GetValue());
			program.SetWorkingDirectory(m_WorkingFolderInput->GetValue());
			program.SetArguments(m_ArgumentsInput->GetValue());
		};

		if (m_Program)
		{
			SetData(*m_Program);
			return *m_Program;
		}
		else
		{
			IProgramEntry& program = IProgramManager::GetInstance()->EmplaceProgram();
			SetData(program);
			return program;
		}
	}
}
