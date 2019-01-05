#pragma once
#include <WinUser.h>
#include "Message.h"
#include <KxFramework/KxFrame.h>

namespace Kortex::IPC
{
	class ProcessingWindow: public KxFrame
	{
		public:
			static wxString GetWindowName();

		private:
			WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override;

		protected:
			virtual void OnMessage(const Message& message) = 0;

		public:
			ProcessingWindow(wxWindow* parent = nullptr);
	};
}
