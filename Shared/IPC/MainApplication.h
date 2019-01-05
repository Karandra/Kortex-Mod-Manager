#pragma once
#include <WinUser.h>
#include "MessageExchanger.h"
#include <KxFramework/KxProcess.h>

namespace Kortex::IPC
{
	class MainApplication: public MessageExchanger
	{
		public:
			MainApplication(HWND windowHandle);
			virtual ~MainApplication();
	};
}
