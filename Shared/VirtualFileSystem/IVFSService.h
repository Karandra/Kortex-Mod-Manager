#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class IVirtualFileSystem;

	class IVFSService: public KxSingletonPtr<IVFSService>
	{
		friend class IVirtualFileSystem;

		protected:
			virtual void RegisterFS(IVirtualFileSystem& vfs) = 0;
			virtual void UnregisterFS(IVirtualFileSystem& vfs) = 0;

		public:
			virtual bool IsOK() const = 0;
			virtual void* GetNativeService() = 0;
			template<class T> T* GetNativeService()
			{
				return static_cast<T*>(GetNativeService());
			}

			virtual wxString GetServiceName() const = 0;
			virtual bool IsInstalled() const = 0;
			virtual bool IsStarted() const = 0;

			virtual bool Start() = 0;
			virtual bool Stop() = 0;
			virtual bool Install() = 0;
			virtual bool Uninstall() = 0;

			virtual bool IsLogEnabled() const = 0;
			virtual void EnableLog(bool value = true) = 0;

		public:
			virtual wxString GetLibraryName() const = 0;
			virtual wxString GetLibraryURL() const = 0;
			virtual KxVersion GetLibraryVersion() const = 0;

			virtual bool HasNativeLibrary() const = 0;
			virtual wxString GetNativeLibraryName() const = 0;
			virtual wxString GetNativeLibraryURL() const = 0;
			virtual KxVersion GetNativeLibraryVersion() const = 0;
	};
}
