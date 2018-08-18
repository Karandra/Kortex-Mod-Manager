#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorWorkspace.h"
#include "KLabeledValue.h"
class KPackageCreatorController;
class KPackageProject;
class KxLabel;
class KxTextBox;

class KPackageCreatorPageBase: public KWorkspace
{
	public:
		static const int ms_LeftMargin = KLC_HORIZONTAL_SPACING * 4;

	protected:
		wxBoxSizer* m_MainSizer = NULL;
		KPackageCreatorWorkspace* m_MainWorkspace = NULL;
		KPackageCreatorController* m_Controller = NULL;

	public:
		KPackageCreatorPageBase(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller);
		virtual ~KPackageCreatorPageBase();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual wxString OnGetWindowTitle() const override;
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

	protected:
		KPackageProject* GetProject() const;

	public:
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_BOX;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		virtual bool ShouldRecieveVFSEvents() const override
		{
			return false;
		}

	public:
		virtual wxString GetName() const override;
		virtual wxString GetPageName() const = 0;
		KPackageCreatorWorkspace* GetMainWorkspace() const
		{
			return m_MainWorkspace;
		}
		KxTextBox* CreateInputField(wxWindow* window);

		static KxLabel* CreateCaptionLabel(wxWindow* window, const wxString& label);
		static KxLabel* CreateNormalLabel(wxWindow* window, const wxString& label, bool addColon = true, bool addLine = false);
		template<class T> static T* AddControlsRow(wxSizer* sizer, const wxString& labelText, T* control, int controlProportion = 1, KxLabel** labelOut = NULL)
		{
			KxLabel* label = CreateNormalLabel(control->GetParent(), labelText);
			if (labelOut)
			{
				*labelOut = label;
			}
			sizer->Add(label, 0, wxEXPAND);

			if (controlProportion == 0)
			{
				wxBoxSizer* controlSizer = new wxBoxSizer(wxVERTICAL);
				controlSizer->Add(control, 0);
				sizer->Add(controlSizer, 1, wxEXPAND);
			}
			else
			{
				sizer->Add(control, controlProportion, wxEXPAND);
			}
			return control;
		}
		template<class T> static T* AddControlsRow2(wxWindow* window, wxSizer* sizer, const wxString& label, T* object, int objectProportion = 1, bool addSpacer = true)
		{
			wxBoxSizer* labelSizer = new wxBoxSizer(wxVERTICAL);
			labelSizer->Add(CreateNormalLabel(window, label), 0, wxEXPAND);
			if (addSpacer)
			{
				labelSizer->AddStretchSpacer(1);
			}

			sizer->Add(labelSizer, 1, wxEXPAND);
			sizer->Add(object, objectProportion, wxEXPAND);
			return object;
		}
		static KxAuiToolBar* CreateListToolBar(wxWindow* window, bool isVertical = false, bool showText = false);
		
		static void ShowTooltipWarning(wxWindow* window, const wxString& message, const wxRect& rect = wxNullRect);
		static void WarnIDCollision(wxWindow* window, const wxRect& rect = wxNullRect);
};
