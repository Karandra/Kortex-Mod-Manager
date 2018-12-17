#pragma once;
#include "stdafx.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxStdDialog.h>
class KModFilesExplorerModel;

namespace Kortex
{
	class IGameMod;
	class FileTreeNode;
}

namespace Kortex::ModManager
{
	class KModFilesExplorerModel;

	class KModFilesExplorerDialog: public KxStdDialog
	{
		private:
			KxPanel* m_ViewPane = nullptr;
			KModFilesExplorerModel* m_ViewModel = nullptr;
			const IGameMod& m_Mod;

			//KProgramOptionAI m_ViewOptions;
			//KProgramOptionAI m_WindowOptions;

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
			virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
			{
				return true;
			}
			virtual wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}

			void CreateUI(wxWindow* parent);

		public:
			KModFilesExplorerDialog(wxWindow* parent, const IGameMod& mod);
			virtual ~KModFilesExplorerDialog();
	};
}
