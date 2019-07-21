#pragma once
#include "stdafx.h"
#include "Application/Options/Macros.h"

namespace Kortex::CmdLineName
{
	KortexDefOption(InstanceID);
	KortexDefOption(DownloadLink);
	KortexDefOption(GlobalConfigPath);
}

namespace Kortex
{
	struct CmdLine
	{
		wxString Executable;
		wxString Arguments;
	};
	struct CmdLineParameters
	{
		wxString InstanceID;
		wxString DownloadLink;
		wxString GlobalConfigPath;
	};
}
