#pragma once
#include "stdafx.h"
#include "IPC/ProcessingWindow.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex::VirtualFileSystem
{
	class DefaultVFSService;

	class RecievingWindow: public IPC::ProcessingWindow, public KxSingletonPtr<RecievingWindow>
	{
		private:
			DefaultVFSService& m_Service;

		protected:
			void OnMessage(const IPC::Message& message) override;

		public:
			RecievingWindow(DefaultVFSService& service, wxWindow* parent = nullptr);
			virtual ~RecievingWindow();

		public:
			bool Destroy() override;
	};
}
