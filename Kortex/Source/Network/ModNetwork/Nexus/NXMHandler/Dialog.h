#pragma once
#include "stdafx.h"
#include "DisplayModel.h"
#include "OptionStore.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxPanel.h>
#include <Kx/System/FileTypeManager.h>

namespace Kortex::NetworkManager
{
	class NexusModNetwork;
}

namespace Kortex::NetworkManager::NXMHandler
{
	class Dialog: public KxStdDialog
	{
		private:
			KxFileTypeManager m_FileTypeManager;
			KxFileType m_NXMFileType;
			OptionStore& m_Options;

			KxPanel* m_Panel = nullptr;
			DisplayModel* m_DisplayModel = nullptr;
			KxLabel* m_RegisteredToLabel = nullptr;
			wxWindow* m_RegisterButton = nullptr;
			wxWindow* m_UnregisterButton = nullptr;

		private:
			bool CreateUI(wxWindow* parent);
			void UpdateButtons();
			AppOption GetOptions() const;

			void OnRegisterAssociations(wxCommandEvent& event);
			void OnUnregisterAssociations(wxCommandEvent& event);

			bool RegisterAssociations();
			bool UnregisterAssociations();
			bool CheckPrimaryHandler();

		private:
			int GetViewSizerProportion() const override
			{
				return 1;
			}
			wxOrientation GetViewSizerOrientation() const override
			{
				return wxVERTICAL;
			}
			wxOrientation GetViewLabelSizerOrientation() const override
			{
				return wxHORIZONTAL;
			}
			bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
			{
				return false;
			}
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_Panel;
			}
			wxWindow* GetDialogFocusCtrl() const override
			{
				return m_DisplayModel->GetView();
			}

		public:
			Dialog(wxWindow* parent, OptionStore& options);
			~Dialog();
	};
}
