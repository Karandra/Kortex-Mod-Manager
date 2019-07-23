#pragma once
#include "stdafx.h"
#include "DisplayModel.h"
#include "OptionStore.h"
#include <KxFramework/KxStdDialog.h>
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

			DisplayModel* m_DisplayModel = nullptr;
			wxWindow* m_RegisterButton = nullptr;
			wxWindow* m_UnregisterButton = nullptr;

		private:
			bool CreateUI(wxWindow* parent);
			void UpdateButtons();
			IAppOption GetOptions() const;

			void OnRegisterAssociations(wxCommandEvent& event);
			void OnUnregisterAssociations(wxCommandEvent& event);

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
				return true;
			}
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_DisplayModel->GetView();
			}

		public:
			Dialog(wxWindow* parent, OptionStore& options);
			~Dialog();
	};
}
