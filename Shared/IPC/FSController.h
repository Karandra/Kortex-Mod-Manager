#pragma once
#include <WinUser.h>
#include "MessageExchanger.h"
#include <KxFramework/KxProcess.h>

namespace Kortex::IPC
{
	class ProcessingWindow;

	class FSController: public MessageExchanger
	{
		private:
			KxProcess m_Process;
			bool m_IsReady = false;

		private:
			void OnProcessReady(wxProcessEvent& event);

		public:
			FSController(const wxString& executablePath);
			virtual ~FSController();

		public:
			bool IsProcessReady() const
			{
				return m_IsReady;
			}
			void SetProcessingWindow(const ProcessingWindow& processingWindow);
			void Run();
	};
}
