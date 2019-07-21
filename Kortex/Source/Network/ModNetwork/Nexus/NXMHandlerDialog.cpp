#include "stdafx.h"
#include "NXMHandlerDialog.h"
#include "Network/INetworkManager.h"

namespace Kortex::NetworkManager
{
	bool NXMHandlerDialog::CreateUI(wxWindow* parent)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("NetworkManager.NXMHandler.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			SetInitialSize(wxSize(840, 470));

			// View
			m_DisplayModel = new NXMHandlerModel();
			m_DisplayModel->CreateView(m_ContentPanel);

			PostCreate(wxDefaultPosition);
			return true;
		}
		return false;
	}
	IAppOption NXMHandlerDialog::GetOptions() const
	{
		return Application::GetGlobalOptionOf<INetworkManager>(m_Nexus.GetName(), "NXMHandler");
	}

	NXMHandlerDialog::NXMHandlerDialog(wxWindow* parent)
		:m_Nexus(*NexusModNetwork::GetInstance())
	{
		CreateUI(parent);
		GetOptions().LoadDataViewLayout(m_DisplayModel->GetView());
	}
	NXMHandlerDialog::~NXMHandlerDialog()
	{
		GetOptions().SaveDataViewLayout(m_DisplayModel->GetView());
	}
}
