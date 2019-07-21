#pragma once
#include "stdafx.h"
#include "Nexus.h"
#include "NXMHandlerModel.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::NetworkManager
{
	class NXMHandlerDialog: public KxStdDialog
	{
		private:
			NexusModNetwork& m_Nexus;
			NXMHandlerModel* m_DisplayModel = nullptr;

		private:
			bool CreateUI(wxWindow* parent);
			IAppOption GetOptions() const;

		private:
			int GetViewSizerProportion() const override
			{
				return 1;
			}
			virtual wxOrientation GetViewSizerOrientation() const override
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
			NXMHandlerDialog(wxWindow* parent);
			~NXMHandlerDialog();
	};
}
