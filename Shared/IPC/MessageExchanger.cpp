#include "stdafx.h"
#include "MessageExchanger.h"
#include <thread>

namespace Kortex::IPC
{
	KxSharedMemoryBuffer MessageExchanger::DoSendMessage(const Message& message, const void* userData, size_t dataSize)
	{
		COPYDATASTRUCT data = {0};
		data.cbData = sizeof(message);
		data.lpData = const_cast<void*>(reinterpret_cast<const void*>(&message));

		// Get or allocate shared buffer and keep object for 'SendMessageW' call
		KxSharedMemoryBuffer sharedBuffer = message.GetSharedBuffer(dataSize);
		if (!sharedBuffer.IsOK())
		{
			sharedBuffer = message.AllocateSharedBuffer(dataSize);
		}

		// If we have some data to send, write it to buffer.
		if (userData && dataSize != 0)
		{
			sharedBuffer.WriteData(userData, dataSize);
		}

		// Send message
		::SendMessageW(m_ProcessingWindow, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&data));

		// Return to caller
		return sharedBuffer;
	}
	KxSharedMemoryBuffer MessageExchanger::DoSendStringMessage(const Message& message, const wxString& value)
	{
		return DoSendMessage(message, value.wc_str(), value.length() * sizeof(wxChar) + sizeof(wxChar));
	}

	void MessageExchanger::Create(HWND windowHandle)
	{
		m_ProcessingWindow = windowHandle;
	}

	MessageExchanger::MessageExchanger(HWND windowHandle)
	{
		Create(windowHandle);
	}

	bool MessageExchanger::IsOK() const
	{
		return m_ProcessingWindow != nullptr;
	}
	
	KxSharedMemoryBuffer MessageExchanger::Send(const Message& messange, const void* userData, size_t dataSize)
	{
		return DoSendMessage(messange, userData, dataSize);
	}
	KxSharedMemoryBuffer MessageExchanger::SendExit()
	{
		return DoSendMessage(RequestID::Exit);
	}
}
