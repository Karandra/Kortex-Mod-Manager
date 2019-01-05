#include "stdafx.h"
#include "Message.h"
#include "ProcessingWindow.h"

namespace Kortex::IPC
{
	wxString ProcessingWindow::GetWindowName()
	{
		return wxS("Kortex::IPC::ProcessingWindow");
	}

	WXLRESULT ProcessingWindow::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
	{
		if (msg == WM_COPYDATA && lParam)
		{
			const COPYDATASTRUCT* copyData = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
			if (copyData->cbData == sizeof(Message))
			{
				OnMessage(*reinterpret_cast<const Message*>(copyData->lpData));
				return true;
			}
		}
		return KxFrame::MSWWindowProc(msg, wParam, lParam);
	}

	ProcessingWindow::ProcessingWindow(wxWindow* parent)
		:KxFrame(parent, wxID_NONE, wxS("Kortex::IPC::ProcessingWindow"))
	{
	}
}
