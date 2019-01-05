#pragma once
#include "stdafx.h"
#include <KxFramework/KxApp.h>

namespace Kortex::VirtualFileSystem
{
	class FSControllerService;
}

namespace Kortex::FSController
{
	class RecievingWindow;
	class MainApplicationLink;

	class Application: public KxApp<wxApp, Application>
	{
		private:
			wxString m_RootFolder;
			wxString m_DataFolder;

			RecievingWindow* m_RecievingWindow = NULL;
			std::unique_ptr<MainApplicationLink> m_MainApp;
			std::unique_ptr<VirtualFileSystem::FSControllerService> m_Service;

		public:
			Application();
			virtual ~Application();

		public:
			virtual bool OnInit() override;
			virtual int OnExit() override;

			virtual bool OnExceptionInMainLoop() override;
			virtual void OnUnhandledException() override;

		public:
			wxString GetDataFolder() const
			{
				return m_DataFolder;
			}
	};
}
