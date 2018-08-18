#pragma once
#include "stdafx.h"
#include <wx/ipc.h>

class KIPC
{
	public:
		static wxString GetHost()
		{
			return "localhost";
		}
		static wxString GetServiceName()
		{
			return "KortexVFS";
		}
		static wxString GetTopic()
		{
			return "VFS";
		}
};
