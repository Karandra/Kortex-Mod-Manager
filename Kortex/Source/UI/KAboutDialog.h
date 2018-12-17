#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxPanel.h>
class KxHTMLWindow;
class wxHtmlLinkEvent;

//////////////////////////////////////////////////////////////////////////
class KAboutInfo
{
	public:
		enum class Type
		{
			App,
			Software,
			Resource,
		};

	private:
		Type m_Type;
		mutable wxString m_License;

	public:
		KAboutInfo(Type type)
			:m_Type(type)
		{
		}

	public:
		wxString GetLocation() const;
		wxString GetLicense() const;
};

//////////////////////////////////////////////////////////////////////////
class KAboutDialog: public KxStdDialog
{
	private:
		KxImageView* m_Logo = nullptr;
		KxAuiNotebook* m_View = nullptr;

		KAboutInfo m_AppInfo;

	private:
		virtual int GetViewSizerProportion() const override
		{
			return 1;
		}
		virtual wxOrientation GetViewSizerOrientation() const override
		{
			return wxHORIZONTAL;
		}
		virtual wxOrientation GetViewLabelSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_View;
		}

	private:
		KxHTMLWindow* CreateHTMLWindow(wxWindow* parent);
		wxSize GetLogoSize() const;

		wxWindow* CreateTab_Info();
		wxWindow* CreateTab_Modules();
		wxWindow* CreateTab_License();

		void OnLinkClick(wxHtmlLinkEvent& event);

	public:
		KAboutDialog(wxWindow* parent);
		virtual ~KAboutDialog();
};
