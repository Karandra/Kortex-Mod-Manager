#pragma once
#include "Framework.hpp"
#include "Application/Options/Macros.h"

namespace Kortex::CmdLineName
{
	Kortex_DefOption(InstanceID);
	Kortex_DefOption(DownloadLink);
	Kortex_DefOption(GlobalConfigPath);
}

namespace Kortex
{
	struct CmdLine final
	{
		kxf::String Executable;
		kxf::String Arguments;
	};
	struct CmdLineParameters final
	{
		kxf::String InstanceID;
		kxf::String DownloadLink;
		kxf::String GlobalConfigPath;
	};
}
