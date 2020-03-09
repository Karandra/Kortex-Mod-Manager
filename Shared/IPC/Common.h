#pragma once
#include <cstdint>

namespace Kortex::IPC
{
	using FSHandle = size_t;

	enum class RequestID: uint32_t
	{
		None = 0,

		Exit,
		UnhandledException,

		Install,
		Uninstall,
		IsInstalled,

		Start,
		Stop,

		IsLogEnabled,
		EnableLog,

		CreateFS,
		DestroyFS,

		FSEnabled,
		FSDisabled,
		FSIsEnabled,

		FSEnable,
		FSDisable,

		FSGetMountPoint,
		FSSetMountPoint,

		FSSetSource,
		FSSetWriteTarget,
		FSBuildFileTree,
		FSAddVirtualFolder,

		FSEnableAsyncIO,
		FSEnableExtendedSecurity,
		FSEnableImpersonateCallerUser,

		FSIsProcessCreatedInVFS,

		GetLibraryName,
		GetLibraryURL,
		GetLibraryVersion,

		HasNativeLibrary,
		GetNativeLibraryName,
		GetNativeLibraryURL,
		GetNativeLibraryVersion,
	};
	enum class FileSystemID
	{
		Mirror,
		Convergence,
		MultiMirror,
	};
}
