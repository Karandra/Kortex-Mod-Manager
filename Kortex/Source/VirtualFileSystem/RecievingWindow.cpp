#include "stdafx.h"
#include "RecievingWindow.h"
#include "DefaultVFSService.h"
#include "UI/KMainWindow.h"

namespace Kortex::VirtualFileSystem
{
	void RecievingWindow::OnMessage(const IPC::Message& message)
	{
		m_Service.OnMessage(message);
	}

	RecievingWindow::RecievingWindow(DefaultVFSService& service, wxWindow* parent)
		:ProcessingWindow(parent), m_Service(service)
	{
	}
	RecievingWindow::~RecievingWindow()
	{
	}

	bool RecievingWindow::Destroy()
	{
		m_Service.OnDestroyRecievingWindow();
		return ProcessingWindow::Destroy();
	}
}

