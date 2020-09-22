#include "pch.hpp"
#include "Log.h"
#include <wx/log.h>

namespace
{
	constexpr wxLogLevel ToWxLogLevel(Kortex::Log::Level level) noexcept
	{
		using namespace Kortex::Log;

		switch (level)
		{
			case Level::Info:
			{
				return wxLOG_Info;
			}
			case Level::Error:
			{
				return wxLOG_Error;
			}
			case Level::Trace:
			{
				return wxLOG_Trace;
			}
			case Level::Debug:
			{
				return wxLOG_Debug;
			}
			case Level::Status:
			{
				return wxLOG_Status;
			}
			case Level::Message:
			{
				return wxLOG_Message;
			}
			case Level::Warning:
			{
				return wxLOG_Warning;
			}
			case Level::Progress:
			{
				return wxLOG_Progress;
			}
			case Level::FatalError:
			{
				return wxLOG_FatalError;
			}
		};
		return wxLOG_Max;
	}
}

namespace Kortex::Log
{
	bool IsEnabled()
	{
		return wxLog::IsEnabled();
	}
	bool IsLevelEnabled(Level level)
	{
		return wxLog::IsLevelEnabled(ToWxLogLevel(level), wxLOG_COMPONENT);
	}
	void LogString(Level level, const kxf::String& value)
	{
		const wxLogLevel logWxLevel = ToWxLogLevel(level);
		if (wxLog::IsLevelEnabled(logWxLevel, wxLOG_COMPONENT))
		{
			wxLogger logger(logWxLevel, __FILE__, __LINE__, __WXFUNCTION__, wxLOG_COMPONENT);
			logger.Log(wxS("%s"), value.GetWxString());
		}
	}
}
