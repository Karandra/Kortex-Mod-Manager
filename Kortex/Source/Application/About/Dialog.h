#pragma once
#include "stdafx.h"
#include "Utility/KImageProvider.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxPanel.h>
class KxHTMLWindow;
class wxHtmlLinkEvent;

namespace Kortex::Application
{
	namespace About
	{
		class DisplayModel;
		class AppNode;
	}

	class AboutDialog: public KxStdDialog
	{
		friend class About::DisplayModel;

		private:
			KxImageView* m_Logo = nullptr;
			KxAuiNotebook* m_TabView = nullptr;

			std::unique_ptr<About::AppNode> m_AppInfo;
			About::DisplayModel* m_DisplayModel = nullptr;
			wxWindow* m_TemporaryTab = nullptr;

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
				return m_TabView;
			}

		private:
			wxSize GetLogoSize() const;
			wxString GetCaption() const;

			wxWindow* CreateTab_Info();
			wxWindow* CreateTab_UsedSoftware();
			wxWindow* CreateTab_License();

			KxHTMLWindow* CreateHTMLWindow();
			void CreateTemporaryTab(wxWindow* window, const wxString& label, const wxBitmap& bitmap = wxNullBitmap);
			void OnTabChanged(wxAuiNotebookEvent& event);
			void OnLinkClicked(wxHtmlLinkEvent& event);

		public:
			AboutDialog(wxWindow* parent);
			virtual ~AboutDialog();
	};
}
