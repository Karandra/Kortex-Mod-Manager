#include "stdafx.h"
#include "Common.h"
#include "FSControllerService.h"
#include "RecievingWindow.h"
#include "IPC/Serialization.h"
#include "Application.h"
#include <VirtualFileSystem/Mirror.h>
#include <VirtualFileSystem/MultiMirror.h>
#include <VirtualFileSystem/Convergence.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFile.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::IPC;

	wxString GetLogPath()
	{
		const auto app = FSController::Application::GetInstance();

		wxString path = app->GetLogFolder() + wxS("\\FileSystemController");
		if (KxSystem::Is64Bit())
		{
			path += wxS(" x64");
		}
		return path + wxS(".log");
	}
	
	KxVFS::KxDynamicStringW GetDriverPath()
	{
		return KxVFS::FileSystemService::GetDokanyDefaultDriverPath();
	}
	KxVFS::KxDynamicStringW GetServiceName()
	{
		return KxVFS::FileSystemService::GetDokanyDefaultServiceName();
	}
}

namespace Kortex::VirtualFileSystem
{
	IPC::FSHandle FSControllerService::CreateFS(IPC::FileSystemID fileSystemID)
	{
		switch (fileSystemID)
		{
			case FileSystemID::Mirror:
			{
				return GetFileSystemHandle(*m_FileSystems.emplace_back(std::make_unique<Mirror>()));
			}
			case FileSystemID::MultiMirror:
			{
				return GetFileSystemHandle(*m_FileSystems.emplace_back(std::make_unique<MultiMirror>()));
			}
			case FileSystemID::Convergence:
			{
				return GetFileSystemHandle(*m_FileSystems.emplace_back(std::make_unique<Convergence>()));
			}
		}
		return 0;
	}
	void FSControllerService::DestroyFS(KxVFS::IFileSystem& fileSystem)
	{
		auto it = std::find_if(m_FileSystems.begin(), m_FileSystems.end(), [&fileSystem](const auto& value)
		{
			return &fileSystem == value.get();
		});
		m_FileSystems.erase(it);
	}
	void FSControllerService::OnMessage(const IPC::Message& message)
	{
		switch (message.GetRequestID())
		{
			case RequestID::Exit:
			{
				// If the window wasn't displayed, it won't cause app exit when closed.
				// So show it once, the window will be destroyed right after anyway.
				m_RecievingWindow->Show();
				m_RecievingWindow->Destroy();
				break;
			}

			case RequestID::IsInstalled:
			{
				message.SerializePayload(IsInstalled());
				break;
			}
			case RequestID::Install:
			{
				message.SerializePayload(Install());
				break;
			}
			case RequestID::Uninstall:
			{
				message.SerializePayload(Uninstall());
				break;
			}

			case RequestID::IsLogEnabled:
			{
				message.SerializePayload(IsLogEnabled());
				break;
			}
			case RequestID::EnableLog:
			{
				auto [value] = message.DeserializePayload<bool>();
				EnableLog(value);
				break;
			}

			case RequestID::Start:
			{
				message.SerializePayload(Start());
				break;
			}
			case RequestID::Stop:
			{
				message.SerializePayload(Stop());
				break;
			}

			case RequestID::CreateFS:
			{
				auto [id] = message.DeserializePayload<FileSystemID>();

				IPC::FSHandle handle = CreateFS(id);
				message.SerializePayload(handle);
				break;
			}
			case RequestID::DestroyFS:
			{
				auto [handle] = message.DeserializePayload<IPC::FSHandle>();
				DestroyFS(GetFileSystemByHandle(handle));
				break;
			}

			case RequestID::FSEnable:
			{
				auto [handle] = message.DeserializePayload<IPC::FSHandle>();

				KxVFS::IFileSystem& vfs = GetFileSystemByHandle(handle);
				message.SerializePayload(vfs.Mount().GetCode());
				break;
			}
			case RequestID::FSDisable:
			{
				auto [handle] = message.DeserializePayload<IPC::FSHandle>();

				KxVFS::IFileSystem& vfs = GetFileSystemByHandle(handle);
				message.SerializePayload(vfs.UnMount());
				break;
			}

			case RequestID::FSGetMountPoint:
			{
				auto [handle] = message.DeserializePayload<IPC::FSHandle>();

				KxVFS::IFileSystem& vfs = GetFileSystemByHandle(handle);
				message.SerializePayload(ToWxString(vfs.GetMountPoint()));
				break;
			}
			case RequestID::FSSetMountPoint:
			{
				auto [handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				KxVFS::IFileSystem& vfs = GetFileSystemByHandle(handle);
				vfs.SetMountPoint(ToKxDynamicStringRef(path));
				break;
			}

			case RequestID::FSSetWriteTarget:
			{
				auto [handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				vfs.SetWriteTarget(ToKxDynamicStringRef(path));
				break;
			}
			case RequestID::FSSetSource:
			{
				auto [handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				vfs.SetSource(ToKxDynamicStringRef(path));
				break;
			}
			case RequestID::FSBuildFileTree:
			{
				auto [handle] = message.DeserializePayload<IPC::FSHandle>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				message.SerializePayload(vfs.BuildFileTree());
				break;
			}
			case RequestID::FSAddVirtualFolder:
			{
				auto [handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				vfs.AddVirtualFolder(ToKxDynamicStringRef(path));
				break;
			}

			case RequestID::FSEnableAsyncIO:
			{
				auto [handle, value] = message.DeserializePayload<IPC::FSHandle, bool>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				vfs.GetIOManager().EnableAsyncIO(value);
				break;
			}
			case RequestID::FSEnableExtendedSecurity:
			{
				auto [handle, value] = message.DeserializePayload<IPC::FSHandle, bool>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				vfs.EnableExtendedSecurity(value);
				break;
			}
			case RequestID::FSEnableImpersonateCallerUser:
			{
				auto [handle, value] = message.DeserializePayload<IPC::FSHandle, bool>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				vfs.EnableImpersonateCallerUser(value);
				break;
			}

			case RequestID::FSIsProcessCreatedInVFS:
			{
				auto [handle, pid] = message.DeserializePayload<IPC::FSHandle, uint32_t>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				message.SerializePayload(vfs.IsProcessCreatedInVFS(pid));
				break;
			}

			case RequestID::GetLibraryName:
			{
				message.SerializePayload(GetLibraryName());
				break;
			}
			case RequestID::GetLibraryURL:
			{
				message.SerializePayload(GetLibraryURL());
				break;
			}
			case RequestID::GetLibraryVersion:
			{
				message.SerializePayload(GetLibraryVersion().ToString());
				break;
			}

			case RequestID::HasNativeLibrary:
			{
				message.SerializePayload(HasNativeLibrary());
				break;
			}
			case RequestID::GetNativeLibraryName:
			{
				message.SerializePayload(GetNativeLibraryName());
				break;
			}
			case RequestID::GetNativeLibraryURL:
			{
				message.SerializePayload(GetNativeLibraryURL());
				break;
			}
			case RequestID::GetNativeLibraryVersion:
			{
				message.SerializePayload(GetNativeLibraryVersion().ToString());
				break;
			}
		};
	}

	FSControllerService::FSControllerService()
		:FileSystemService(::GetServiceName()), m_Logger(ToKxDynamicStringRef(GetLogPath()))
	{
		m_Logger.Log(KxVFS::LogLevel::Info, L"Log opened");
	}
	FSControllerService::~FSControllerService()
	{
		m_Logger.Log(KxVFS::LogLevel::Info, L"Log closed");
	}

	bool FSControllerService::Install()
	{
		if (!FileSystemService::IsDokanyDefaultInstallPresent())
		{
			return FileSystemService::Install(GetDriverPath(), wxS("Dokany File System Driver"), wxS("Dokan kernel-mode file system driver"));
			
		}
		return true;
	}
	bool FSControllerService::Uninstall()
	{
		return KxVFS::FileSystemService::Uninstall();
	}

	IPC::FSHandle FSControllerService::GetFileSystemHandle(const KxVFS::IFileSystem& fileSystem) const
	{
		return reinterpret_cast<IPC::FSHandle>(&fileSystem);
	}
	KxVFS::IFileSystem& FSControllerService::GetFileSystemByHandle(IPC::FSHandle handle) const
	{
		return *reinterpret_cast<KxVFS::IFileSystem*>(handle);
	}
}
