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
			KxSharedMemoryBuffer DoSendStringMessage(const Message& message, const wxString& value);
			
			template<class T>
			KxSharedMemoryBuffer DoSendValueMessage(const Message& message, const T& value)
			{
				if constexpr(std::is_same_v<T, wxString>)
				{
					return DoSendStringMessage(message, value);
				}
				else
				{
					static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

					return DoSendMessage(message, &value, sizeof(T));
				}
			}
			
			virtual void OnSendMessage(const Message& message, const void* userData, size_t dataSize)
			{
			}
			virtual void OnMessageSent(const Message& message, const void* userData, size_t dataSize, const KxSharedMemoryBuffer& returnValue)
			{
			}

		protected:
			void Create(HWND windowHandle);

		public:
			MessageExchanger() = default;
			MessageExchanger(HWND windowHandle);
			virtual ~MessageExchanger() = default;

		public:
			bool IsOK() const;

			KxSharedMemoryBuffer SendExit();
			KxSharedMemoryBuffer Send(const Message& message, const void* userData = nullptr, size_t dataSize = 0);
			
			template<class... Args>
			KxSharedMemoryBuffer Send(const Message& message, Args&&... arg)
			{
				constexpr size_t count = sizeof...(Args);
				if constexpr(count == 0)
				{
					return DoSendMessage(message);
				}
				else if constexpr(count == 1)
				{
					return DoSendValueMessage(message, arg...);
				}
				else
				{
					return DoSendStringMessage(message, IPC::Serializer::Serialize(std::forward<Args>(arg)...));
				}
			}
			
			template<class T>
			KxSharedMemoryBuffer Send(const Message& message, const T& value)
			{
				return DoSendValueMessage(message, value);
			}
	};
}
