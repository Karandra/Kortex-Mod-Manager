#pragma once
#include <WinUser.h>
#include "Message.h"
#include <KxFramework/KxSharedMemory.h>

namespace Kortex::IPC
{
	class MessageExchanger
	{
		private:
			HWND m_ProcessingWindow = nullptr;

		protected:
			KxSharedMemoryBuffer DoSendMessage(const Message& message, const void* userData = nullptr, size_t dataSize = 0);
			void Create(HWND windowHandle);

		public:
			MessageExchanger() = default;
			MessageExchanger(HWND windowHandle);
			virtual ~MessageExchanger() = default;

		public:
			bool IsOK() const;

			KxSharedMemoryBuffer Send(const Message& message, const void* userData = nullptr, size_t dataSize = 0);
			KxSharedMemoryBuffer SendExit();

			template<class T> KxSharedMemoryBuffer Send(const Message& message, const T& value)
			{
				static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

				return DoSendMessage(message, &value, sizeof(T));
			}
			template<> KxSharedMemoryBuffer Send(const Message& message, const wxString& value)
			{
				return DoSendMessage(message, value.wc_str(), value.length() * sizeof(wxChar) + sizeof(wxChar));
			}
	};
}
