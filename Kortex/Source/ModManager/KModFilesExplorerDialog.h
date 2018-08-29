#pragma once;
#include "stdafx.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxStdDialog.h>
#include "KProgramOptions.h"
class KModEntry;
class KModFilesExplorerModel;

class KModFilesExplorerDialog: public KxStdDialog
{
	private:
		KxPanel* m_ViewPane = NULL;
		KModFilesExplorerModel* m_ViewModel = NULL;
		const KModEntry& m_ModEntry;

		KProgramOptionUI m_ViewOptions;
		KProgramOptionUI m_WindowOptions;

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
			return wxHORIZONTAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = NULL) const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}

		void CreateUI(wxWindow* parent);

	public:
		KModFilesExplorerDialog(wxWindow* parent, const KModEntry& modEntry);
		virtual ~KModFilesExplorerDialog();
};
