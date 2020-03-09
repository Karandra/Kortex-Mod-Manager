#pragma once
#include <WinUser.h>
#include "Common.h"
#include "IPC/Serialization.h"
#include <KxFramework/KxSharedMemory.h>

namespace Kortex::IPC
{
	class Message final
	{
		private:
			using Protection = KxSharedMemoryNS::Protection;

			struct InternalData
			{
				wchar_t SharedMemoryRegionName[48] = {0};
				RequestID RequestID = RequestID::None;
			};

		private:
			static const Protection ms_SharedBufferProtection = Protection::RW;
			static const size_t ms_MinBufferSize = 1024;

		private:
			const wchar_t* GetSharedMemoryRegionName() const
			{
				return m_Data.SharedMemoryRegionName;
			}
			KxSharedMemoryBuffer GetBuffer(size_t requiredSize) const
			{
				KxSharedMemoryBuffer sharedBuffer;
				if (!sharedBuffer.Open(m_Data.SharedMemoryRegionName, requiredSize, Protection::RW))
				{
					sharedBuffer.Allocate(requiredSize, Protection::RW, m_Data.SharedMemoryRegionName);
				}
				else if (sharedBuffer.GetSize() < requiredSize)
				{
					sharedBuffer.Allocate(requiredSize, Protection::RW, m_Data.SharedMemoryRegionName);
				}
				return sharedBuffer;
			}

		private:
			InternalData m_Data;

		public:
			Message(RequestID id = RequestID::None)
			{
				static_assert(std::is_trivially_copyable_v<Message>, "IPC::Message must be trivially copyable");
				static_assert(sizeof(Message) <= 128, "IPC::Message is too big");

				swprintf_s(m_Data.SharedMemoryRegionName, L"Kortex/FSController/0x%p", this);
				SetRequestID(id);
			}

		public:
			RequestID GetRequestID() const
			{
				return m_Data.RequestID;
			}
			void SetRequestID(RequestID id)
			{
				m_Data.RequestID = id;
			}

		public:
			KxSharedMemoryBuffer AllocateSharedBuffer(size_t requiredSize = 0) const
			{
				return KxSharedMemoryBuffer(std::max(ms_MinBufferSize, requiredSize), ms_SharedBufferProtection, m_Data.SharedMemoryRegionName);
			}
			KxSharedMemoryBuffer GetSharedBuffer(size_t requiredSize = 0) const
			{
				KxSharedMemoryBuffer sharedBuffer;
				if (sharedBuffer.Open(m_Data.SharedMemoryRegionName, ms_MinBufferSize, ms_SharedBufferProtection))
				{
					if (requiredSize != 0 && sharedBuffer.GetSize() < std::max(ms_MinBufferSize, requiredSize))
					{
						// Reallocated using same name if we need more space
						sharedBuffer.Allocate(requiredSize, ms_SharedBufferProtection, m_Data.SharedMemoryRegionName);
					}
				}
				return sharedBuffer;
			}

			template<class T>
			T GetPayload() const
			{
				return GetSharedBuffer().GetAs<T>();
			}
			
			template<class T>
			void SetPayload(const T& payload) const
			{
				GetSharedBuffer(sizeof(T)).WriteData(payload);
			}
			
			template<>
			void SetPayload(const wxString& payload) const
			{
				GetSharedBuffer(payload.length() * sizeof(wxChar) + sizeof(wxChar)).WriteData(payload);
			}

			template<class... Args>
			std::tuple<Args...> DeserializePayload() const
			{
				if constexpr((sizeof...(Args)) <= 1)
				{
					return GetPayload<Args...>();
				}
				else
				{
					return IPC::Serializer::Deserialize<Args...>(GetPayload<wxString>());
				}
			}
			
			template<class... Args>
			void SerializePayload(Args&&... arg) const
			{
				if constexpr((sizeof...(Args)) <= 1)
				{
					SetPayload(arg...);
				}
				else
				{
					SetPayload(IPC::Serializer::Serialize(std::forward<Args>(arg)...).ToString());
				}
			}
	};
}
