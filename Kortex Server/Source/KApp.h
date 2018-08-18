#pragma once
#include "stdafx.h"
#include "Profile/KProfileID.h"
class KIPCServer;

class KApp: public KxApp<wxApp, KApp>
{
	private:
		KIPCServer* m_Server = NULL;

	public:
		KApp();
		virtual ~KApp();

	public:
		virtual bool OnInit() override;
		virtual int OnExit() override;
		virtual bool OnExceptionInMainLoop() override;
		virtual void OnUnhandledException() override;

	public:
		const wxString& GetDataFolder();
		
		KIPCServer* GetServer() const
		{
			return m_Server;
		}
};
