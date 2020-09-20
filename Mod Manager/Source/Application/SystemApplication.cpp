#include "pch.h"
#include "SystemApplication.h"
#include "kxf/Application/ApplicationInitializer.h"
#include <wx/msgdlg.h>

namespace Kortex
{
	bool SystemApplication::OnCreate()
	{
		return true;
	}
	bool SystemApplication::OnInit()
	{
		return true;
	}

	bool SystemApplication::OnMainLoopException()
	{
		wxMessageBox(__FUNCTION__);
		return false;
	}
	void SystemApplication::OnUnhandledException()
	{
		wxMessageBox(__FUNCTION__);
	}
	void SystemApplication::OnAssertFailure(kxf::String file, int line, kxf::String function, kxf::String condition, kxf::String message)
	{
		wxMessageBox("OnAssertFailure", file);
	}
}

int main(int argc, char** argv)
{
	Kortex::SystemApplication app;
	return kxf::ApplicationInitializer(app, argc, argv).Run();
}
