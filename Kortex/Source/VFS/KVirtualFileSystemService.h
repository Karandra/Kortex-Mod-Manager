#pragma once
#include "stdafx.h"

class KxVFSService;
class KVirtualFileSystemService
{
	public:
		static wxString GetLibraryVersion();
		static wxString GetDokanVersion();

	private:
		static wxString InitLibraryPath();
		static wxString InitDriverPath();

	private:
		const wxString m_Name;
		const wxString m_DisplayName;
		const wxString m_Description;
		const wxString m_LibraryPath;
		const wxString m_DriverPath;
		KxVFSService* m_ServiceImpl = NULL;
		HMODULE m_LibraryHandle = NULL;

	private:
		void UnInit();

	public:
		KVirtualFileSystemService();
		virtual ~KVirtualFileSystemService();
		bool Init();

	public:
		bool IsOK() const;

		const wxString& GetName() const
		{
			return m_Name;
		}
		const wxString& GetDisplayName() const
		{
			return m_DisplayName;
		}
		const wxString& GetDescription() const
		{
			return m_Description;
		}
		const wxString& GetLibraryPath() const
		{
			return m_LibraryPath;
		}
		const wxString& GetDriverPath() const
		{
			return m_DriverPath;
		}

		KxVFSService* GetServiceImpl() const;
		bool IsReady() const;
		bool IsStarted() const;
		bool IsInstalled() const;

		bool Start();
		bool Stop();
		bool Install();
		bool Uninstall();
};
