#include "stdafx.h"
#include "MainApplication.h"

namespace Kortex::IPC
{
	MainApplication::MainApplication(HWND windowHandle)
		:MessageExchanger(windowHandle)
	{
	}
	MainApplication::~MainApplication()
	{
	}
}
