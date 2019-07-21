#include "stdafx.h"
#include "NXMHandlerDialog.h"

namespace Kortex::NetworkManager
{
	bool NXMHandlerDialog::CreateUI(wxWindow* parent)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("NetworkManager.NXMHandler.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			SetInitialSize(wxSize(740, 320));

			// View
			m_DisplayModel = new NXMHandlerModel();
			m_DisplayModel->CreateView(m_ContentPanel);

			PostCreate(wxDefaultPosition);
			return true;
		}
		return false;
	}
}
