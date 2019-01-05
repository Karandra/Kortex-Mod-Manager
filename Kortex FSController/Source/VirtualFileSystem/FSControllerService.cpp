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

#if defined _WIN64
#pragma comment(lib, "KxVirtualFileSystem x64.lib")
#else
#pragma comment(lib, "KxVirtualFileSystem.lib")
#endif

namespace
{
	using namespace Kortex;
	using namespace Kortex::IPC;

	wxString GetDriverPath()
	{
		const auto app = FSController::Application::GetInstance();

		wxString path = app->GetDataFolder() + wxS("\\VFS\\Drivers\\");
		if (KxSystem::IsWindows10OrGreater())
		{
			path += wxS("Win10");
		}
		else if (KxSystem::IsWindows8Point1OrGreater())
		{
			path += wxS("Win8.1");
		}
		else if (KxSystem::IsWindows8OrGreater())
		{
			path += wxS("Win8");
		}
		else
		{
			path += wxS("Win7");
		}

		if (KxSystem::Is64Bit())
		{
			path += wxS(" x64");
		}
		path += wxS("\\dokan2.sys");

		KxFile filePath(path);
		if (filePath.IsFileExist())
		{
			return filePath.GetFullPath();
		}
		return wxEmptyString;
	}
}

namespace Kortex::VirtualFileSystem
{
	void FSControllerService::OnMessage(const IPC::Message& message)
	{
		switch (message.GetRequestID())
		{
			case RequestID::Exit:
			{
				// If the window wasn't displayed, it won't cause app exit when closed.
				// So show it once,the window will be destroyed right after anyway.
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
				auto[id] = message.DeserializePayload<FileSystemID>();

				IPC::FSHandle handle = OnCreateFS(id);
				message.SerializePayload(handle);
				break;
			}
			case RequestID::DestroyFS:
			{
				auto[id] = message.DeserializePayload<FileSystemID>();

				IPC::FSHandle handle = OnCreateFS(id);
				DestroyFS(GetFileSystemByHandle(handle));
				break;
			}

			case RequestID::FSEnable:
			{
				auto[handle] = message.DeserializePayload<IPC::FSHandle>();

				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(handle);
				message.SerializePayload(vfs.Mount().GetCode());
				break;
			}
			case RequestID::FSDisable:
			{
				auto[handle] = message.DeserializePayload<IPC::FSHandle>();

				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(handle);
				message.SerializePayload(vfs.UnMount());
				break;
			}

			case RequestID::FSSetMountPoint:
			{
				auto[handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(handle);
				vfs.SetMountPoint(ToKxDynamicStringRef(path));
				break;
			}
			case RequestID::FSSetWriteTarget:
			{
				auto[handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				vfs.SetWriteTarget(ToKxDynamicStringRef(path));
				break;
			}
			case RequestID::FSSetSource:
			{
				auto[handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				vfs.SetSource(ToKxDynamicStringRef(path));
				break;
			}
			case RequestID::FSBuildDispatcherIndex:
			{
				auto[handle] = message.DeserializePayload<IPC::FSHandle>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				message.SerializePayload(vfs.BuildDispatcherIndex());
				break;
			}
			case RequestID::FSAddVirtualFolder:
			{
				auto[handle, path] = message.DeserializePayload<IPC::FSHandle, wxString>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				vfs.AddVirtualFolder(ToKxDynamicStringRef(path));
				break;
			}

			case RequestID::FSEnableINIOptimization:
			{
				auto[handle, value] = message.DeserializePayload<IPC::FSHandle, bool>();

				auto& vfs = GetFileSystemByHandle<KxVFS::ConvergenceFS>(handle);
				vfs.EnableINIOptimization(value);
				break;
			}
			case RequestID::FSEnableSecurityFunctions:
			{
				auto[handle, value] = message.DeserializePayload<IPC::FSHandle, bool>();

				auto& vfs = GetFileSystemByHandle<KxVFS::MirrorFS>(handle);
				vfs.EnableSecurityFunctions(value);
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
	IPC::FSHandle FSControllerService::OnCreateFS(IPC::FileSystemID fileSystemID)
	{
		switch (fileSystemID)
		{
			case FileSystemID::Mirror:
			{
				auto& vfs = m_FileSystems.emplace_back(std::make_unique<Mirror>());
				return GetFileSystemHandle(*vfs);
			}
			case FileSystemID::MultiMirror:
			{
				auto& vfs = m_FileSystems.emplace_back(std::make_unique<MultiMirror>());
				return GetFileSystemHandle(*vfs);
			}
			case FileSystemID::Convergence:
			{
				auto& vfs = m_FileSystems.emplace_back(std::make_unique<Convergence>());
				return GetFileSystemHandle(*vfs);
			}
		}
		return 0;
	}

	void FSControllerService::DestroyFS(KxVFS::AbstractFS& vfs)
	{
		auto it = std::find_if(m_FileSystems.begin(), m_FileSystems.end(), [&vfs](const auto& value)
		{
			return &vfs == value.get();
		});
		m_FileSystems.erase(it);
	}

	FSControllerService::FSControllerService()
		:Service(L"KortexVFS")
	{
	}
	FSControllerService::~FSControllerService()
	{
	}

	bool FSControllerService::Install()
	{
		const constexpr wxChar name[] = wxS("Kortex Mod Manager");
		wxString displayName = KxString::Format("%1 Virtual File System Service", name);
		wxString description = KxString::Format("The VFS service provides support for the virtual file system for %1", name);

		return KxVFS::Service::Install(ToKxDynamicStringRef(GetDriverPath()), ToKxDynamicStringRef(displayName), ToKxDynamicStringRef(description));
	}
	bool FSControllerService::Uninstall()
	{
		return KxVFS::Service::Uninstall();
	}

	IPC::FSHandle FSControllerService::GetFileSystemHandle(const KxVFS::AbstractFS& vfs) const
	{
		return reinterpret_cast<IPC::FSHandle>(&vfs);
	}
	KxVFS::AbstractFS& FSControllerService::GetFileSystemByHandle(IPC::FSHandle handle) const
	{
		return *reinterpret_cast<KxVFS::AbstractFS*>(handle);
	}
}
