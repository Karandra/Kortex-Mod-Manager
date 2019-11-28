#include "stdafx.h"
#include "PageBase.h"
#include "PackageProject/ModPackageProject.h"
#include "Workspace.h"
#include "WorkspaceDocument.h"
#include "ModPackages/IPackageManager.h"
#include "GameInstance/IGameInstance.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxStdDialogSimple.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxRichToolTip.h>

namespace Kortex::PackageDesigner
{
	bool PageBase::OnCreateWorkspace()
	{
		return true;
	}
	bool PageBase::OnOpenWorkspace()
	{
		return true;
	}
	bool PageBase::OnCloseWorkspace()
	{
		return true;
	}
	void PageBase::OnReloadWorkspace()
	{
	}

	PageBase::PageBase(Workspace& mainWorkspace, WorkspaceDocument& controller)
		:m_MainWorkspace(&mainWorkspace), m_Controller(&controller)
	{
	}

	ModPackageProject* PageBase::GetProject() const
	{
		return m_Controller->GetProject();
	}
	wxString PageBase::GetName() const
	{
		return GetPageName();
	}

	KxTextBox* PageBase::CreateInputField(wxWindow* window)
	{
		KxTextBox* textBox = new KxTextBox(window, KxID_NONE);
		textBox->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
		{
			m_Controller->ChangeNotify();
			event.Skip();
		});
		return textBox;
	}

	KxLabel* PageBase::CreateCaptionLabel(wxWindow* window, const wxString& label)
	{
		return new KxLabel(window, KxID_NONE, label, KxLabel::DefaultStyle|KxLABEL_CAPTION|KxLABEL_LINE|KxLABEL_COLORED);
	}
	KxLabel* PageBase::CreateNormalLabel(wxWindow* window, const wxString& label, bool addColon, bool addLine)
	{
		int style = (addLine ? KxLABEL_LINE : 0);
		return new KxLabel(window, KxID_NONE, !addColon || label.IsEmpty() ? label : label + ':', style);
	}
	KxAuiToolBar* PageBase::CreateListToolBar(wxWindow* window, bool isVertical, bool showText)
	{
		int flags = KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND|(isVertical ? wxAUI_TB_VERTICAL : wxAUI_TB_HORIZONTAL)|(showText ? wxAUI_TB_TEXT : 0);

		KxAuiToolBar* toolBar = new KxAuiToolBar(window, KxID_NONE, flags);
		toolBar->SetBackgroundColour(window->GetBackgroundColour());
		toolBar->SetToolPacking(0);
		toolBar->SetMargins(0, 0, 0, 0);
		toolBar->SetToolSeparation(0);

		Kortex::IThemeManager::GetActive().Apply(static_cast<wxWindow*>(toolBar));
		return toolBar;
	}
	void PageBase::ShowTooltipWarning(wxWindow* window, const wxString& message, const wxRect& rect)
	{
		KxRichToolTip tooltip(KTr(KxID_ERROR), message);
		tooltip.SetIcon(KxICON_WARNING);

		if (!rect.IsEmpty() || wxRect(wxPoint(0, 0), wxGetDisplaySize()).Contains(rect))
		{
			tooltip.SetKind(wxTipKind_BottomLeft);
		}
		tooltip.Show(window, rect);
	}
	void PageBase::WarnIDCollision(wxWindow* window, const wxRect& rect)
	{
		ShowTooltipWarning(window, KTr("PackageCreator.IDCollision"), rect);
	}
}
