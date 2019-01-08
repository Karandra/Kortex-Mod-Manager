#pragma once
#include "stdafx.h"
#include <KxFramework/KxString.h>

namespace Kortex::Utility::Log
{
	enum class Level
	{
		Info,
		Error,
		Trace,
		Debug,
		Status,
		Message,
		Warning,
		Progress,
		FatalError,
	};
}

namespace Kortex::Utility::Log
{
	bool IsEnabled();
	bool IsLevelEnabled(Level level);
	void LogString(Level level, const wxString& value);
}

namespace Kortex::Utility::Log
{
	namespace Internal
	{
		inline void LogIfEnabled(Level level, const wxString& format)
		{
			if (IsLevelEnabled(level))
			{
				LogString(level, format);
			}
		}
		template<class... Args> void LogIfEnabled(Level level, const wxString& format, Args&&... arg)
		{
			if (IsLevelEnabled(level))
			{
				LogString(level, KxString::Format(format, std::forward<Args>(arg)...));
			}
		}
	}

	template<class... Args> void LogInfo(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Info, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogError(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Error, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogTrace(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Trace, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogDebug(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Debug, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogStatus(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Status, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogMessage(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Message, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogWarning(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Warning, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogProgress(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::Progress, format, std::forward<Args>(arg)...);
	}
	template<class... Args> void LogFatalError(const wxString& format, Args&&... arg)
	{
		Internal::LogIfEnabled(Level::FatalError, format, std::forward<Args>(arg)...);
	}
}
