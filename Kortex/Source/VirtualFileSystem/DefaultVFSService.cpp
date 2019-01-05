#include "stdafx.h"
#include "DefaultVFSService.h"
#include "RecievingWindow.h"
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "Application/SystemApplication.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxFile.h>

namespace
{
	using namespace Kortex::IPC;

	wxString GetFSControllerPath()
	{
		const auto app = Kortex::IApplication::GetInstance();
		if (app->IsSystem64Bit())
		{
			return app->GetRootFolder() + wxS("\\FileSystemController x64.exe");
		}
		else
		{
			return app->GetRootFolder() + wxS("\\FileSystemController.exe");
		}
	}
}

namespace Kortex::VirtualFileSystem
{
	void DefaultVFSService::OnDestroyRecievingWindow()
	{
		m_RecievingWindow = nullptr;
	}
	void DefaultVFSService::OnMessage(const IPC::Message& message)
	{
		switch (message.GetRequestID())
		{
			case RequestID::Exit:
			{
				if (KMainWindow* mainWindow = KMainWindow::GetInstance())
				{
					mainWindow->Close();
				}
				break;
			}
			case RequestID::UnhandledException:
			{
				auto [exceptionMessage] = message.DeserializePayload<wxString>();

				KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTr("VFS.Service.UnhandledException"), exceptionMessage, KxBTN_OK, KxICON_ERROR);
				dialog.ShowModal();

				Stop();
				Start();
				break;
			}

			case RequestID::FSEnabled:
			{
				if (IVirtualFileSystem* vfs = FindFSByhandle(message.GetPayload<IPC::FSHandle>()))
				{
					IEvent::MakeQueue<VFSEvent>(Events::SingleVFSToggled, *vfs, true);
				}
				break;
			}
			case RequestID::FSDisabled:
			{
				if (IVirtualFileSystem* vfs = FindFSByhandle(message.GetPayload<IPC::FSHandle>()))
				{
					IEvent::MakeQueue<VFSEvent>(Events::SingleVFSToggled, *vfs, false);
				}
				break;
			}
		};
	}
	void DefaultVFSService::RunController()
	{
		if (!m_RecievingWindow)
		{
			m_RecievingWindow = new RecievingWindow(*this);
			KMainWindow::GetInstance()->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evnet)
			{
				Stop();
				evnet.Skip();
			});
			
			m_Controller.SetProcessingWindow(*m_RecievingWindow);
			m_Controller.Run();
		}
	}

	IVirtualFileSystem* DefaultVFSService::FindFSByhandle(IPC::FSHandle handle) const
	{
		auto it = std::find_if(m_FileSystems.begin(), m_FileSystems.end(), [handle](const IVirtualFileSystem* value)
		{
			return value->GetHandle() == handle;
		});
		return it != m_FileSystems.end() ? *it : nullptr;
	}

	DefaultVFSService::DefaultVFSService()
		:m_Controller(GetFSControllerPath())
	{
		RunController();
	}
	DefaultVFSService::~DefaultVFSService()
	{
		m_Controller.SendExit();
	}

	bool DefaultVFSService::IsOK() const
	{
		return m_Controller.IsOK() && m_Controller.IsProcessReady();
	}

	bool DefaultVFSService::IsInstalled() const
	{
		return m_Controller.Send(RequestID::IsInstalled).GetAs<bool>();
	}
	bool DefaultVFSService::IsStarted() const
	{
		return m_RecievingWindow && IsOK();
	}

	bool DefaultVFSService::Start()
	{
		RunController();
		if (IsOK())
		{
			return m_Controller.Send(RequestID::Start).GetAs<bool>();
		}
		return false;
	}
	bool DefaultVFSService::Stop()
	{
		if (m_RecievingWindow)
		{
			m_RecievingWindow->Destroy();
			return m_Controller.Send(RequestID::Stop).GetAs<bool>();
		}
		return false;
	}
	
	bool DefaultVFSService::Install()
	{
		return m_Controller.Send(RequestID::Install).GetAs<bool>();
	}
	bool DefaultVFSService::Uninstall()
	{
		return m_Controller.Send(RequestID::Uninstall).GetAs<bool>();
	}

	wxString DefaultVFSService::GetLibraryName() const
	{
		return m_Controller.Send(RequestID::GetLibraryName).GetAs<wxString>();
	}
	wxString DefaultVFSService::GetLibraryURL() const
	{
		return m_Controller.Send(RequestID::GetLibraryURL).GetAs<wxString>();
	}
	KxVersion DefaultVFSService::GetLibraryVersion() const
	{
		return m_Controller.Send(RequestID::GetLibraryVersion).GetAs<wxString>();
	}

	bool DefaultVFSService::HasNativeLibrary() const
	{
		return m_Controller.Send(RequestID::HasNativeLibrary).GetAs<bool>();
	}
	wxString DefaultVFSService::GetNativeLibraryName() const
	{
		return m_Controller.Send(RequestID::GetNativeLibraryName).GetAs<wxString>();
	}
	wxString DefaultVFSService::GetNativeLibraryURL() const
	{
		return m_Controller.Send(RequestID::GetNativeLibraryURL).GetAs<wxString>();
	}
	KxVersion DefaultVFSService::GetNativeLibraryVersion() const
	{
		return m_Controller.Send(RequestID::GetNativeLibraryVersion).GetAs<wxString>();
	}
}
