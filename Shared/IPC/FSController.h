#pragma once
#include <WinUser.h>
#include "MessageExchanger.h"
#include <KxFramework/KxProcess.h>
#include <condition_variable>

namespace Kortex::IPC
{
	class ProcessingWindow;

	class FSController: public MessageExchanger
	{
		private:
			KxProcess m_Process;
			bool m_IsProcessIdle = false;

			HANDLE m_ProcessHandle = nullptr;
			std::mutex m_ThreadMutex;
			std::condition_variable m_ThreadCondition;

		private:
			void OnProcessReady(wxProcessEvent& event);
			void EndWaitForTermination();
			bool Reset();

		protected:
			void OnSendMessage(const Message& message, const void* userData, size_t dataSize) override;

		public:
			FSController(const wxString& executablePath);
			virtual ~FSController();

		public:
			bool IsRunning() const;
			bool IsProcessReady() const
			{
				return m_IsProcessIdle;
			}

			const KxProcess& GetProcess() const
			{
				return m_Process;
			}
			KxProcess& GetProcess()
			{
				return m_Process;
			}

			void SetProcessingWindow(const ProcessingWindow& processingWindow);
			void Run();
			bool WaitForTermination(std::function<void()> func);
	};
}
