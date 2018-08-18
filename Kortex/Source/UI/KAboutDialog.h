#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxPanel.h>
class wxHtmlLinkEvent;

class KAboutDialog: public KxStdDialog
{
	private:
		KxAuiNotebook* m_View = NULL;

	private:
		virtual int GetViewSizerProportion() const override
		{
			return 1;
		}
		virtual wxOrientation GetViewSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual wxOrientation GetViewLabelSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = NULL) const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_View;
		}
		virtual void ResetState()
		{
		}

	private:
		wxWindow* CreateTab_Info();
		wxWindow* CreateTab_Modules();
		wxWindow* CreateTab_Permissions();

		void OnLinkClick(wxHtmlLinkEvent& event);

	public:
		KAboutDialog(wxWindow* parent);
		virtual ~KAboutDialog();
};
