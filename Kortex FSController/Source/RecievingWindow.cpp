#include "stdafx.h"
#include "RecievingWindow.h"
#include "VirtualFileSystem/FSControllerService.h"

using namespace Kortex::IPC;

namespace Kortex::FSController
{
	void RecievingWindow::OnMessage(const Message& message)
	{
		m_Service.OnMessage(message);
	}

	RecievingWindow::RecievingWindow(VirtualFileSystem::FSControllerService& service, wxWindow* parent)
		:ProcessingWindow(parent), m_Service(service)
	{
	}
	RecievingWindow::~RecievingWindow()
	{
	}
}
