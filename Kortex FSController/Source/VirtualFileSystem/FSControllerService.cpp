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

			case RequestID::CreateFS:
			{
				IPC::FSHandle handle = OnCreateFS(message.GetPayload<FileSystemID>());
				message.SetPayload(handle);
				break;
			}
			case RequestID::DestroyFS:
			{
				IPC::FSHandle handle = OnCreateFS(message.GetPayload<FileSystemID>());
				DestroyFS(GetFileSystemByHandle(handle));
				break;
			}

			case RequestID::FSEnable:
			{
				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(message.GetPayload<IPC::FSHandle>());
				message.SetPayload(vfs.Mount().GetCode());
				break;
			}
			case RequestID::FSDisable:
			{
				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(message.GetPayload<IPC::FSHandle>());
				message.SetPayload(vfs.UnMount());
				break;
			}

			case RequestID::FSSetMountPoint:
			{
				IPC::Serializer serializer = message.DeserializePayload();

				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(serializer.GetInt<IPC::FSHandle>(0));
				vfs.SetMountPoint(ToKxDynamicStringRef(serializer.GetString(1)));
				break;
			}
			case RequestID::FSSetWriteTarget:
			case RequestID::FSSetSource:
			{
				// Implementation detail: source and write target is the same things in Convergence and Mirror.

				IPC::Serializer serializer = message.DeserializePayload();

				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(serializer.GetInt<IPC::FSHandle>(0));
				static_cast<KxVFS::MirrorFS&>(vfs).SetSource(ToKxDynamicStringRef(serializer.GetString(1)));
				break;
			}
			case RequestID::FSBuildDispatcherIndex:
			{
				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(message.GetPayload<IPC::FSHandle>());
				size_t count = static_cast<KxVFS::ConvergenceFS&>(vfs).BuildDispatcherIndex();
				message.SetPayload(count);
				break;
			}
			case RequestID::FSAddVirtualFolder:
			{
				IPC::Serializer serializer = message.DeserializePayload();

				KxVFS::AbstractFS& vfs = GetFileSystemByHandle(serializer.GetInt<IPC::FSHandle>(0));
				KxVFS::ConvergenceFS& convergence = static_cast<KxVFS::ConvergenceFS&>(vfs);
				convergence.AddVirtualFolder(ToKxDynamicStringRef(serializer.GetString(1)));
				break;
			}

			case RequestID::IsInstalled:
			{
				message.SetPayload(IsInstalled());
				break;
			}
			case RequestID::Install:
			{
				message.SetPayload(Install());
				break;
			}
			case RequestID::Uninstall:
			{
				message.SetPayload(Uninstall());
				break;
			}

			case RequestID::GetLibraryName:
			{
				message.SetPayload(GetLibraryName());
				break;
			}
			case RequestID::GetLibraryURL:
			{
				message.SetPayload(GetLibraryURL());
				break;
			}
			case RequestID::GetLibraryVersion:
			{
				message.SetPayload(GetLibraryVersion().ToString());
				break;
			}

			case RequestID::HasNativeLibrary:
			{
				message.SetPayload(HasNativeLibrary());
				break;
			}
			case RequestID::GetNativeLibraryName:
			{
				message.SetPayload(GetNativeLibraryName());
				break;
			}
			case RequestID::GetNativeLibraryURL:
			{
				message.SetPayload(GetNativeLibraryURL());
				break;
			}
			case RequestID::GetNativeLibraryVersion:
			{
				message.SetPayload(GetNativeLibraryVersion().ToString());
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
