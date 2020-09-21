#pragma once
#include "Framework.hpp"

namespace Kortex::Log
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

namespace Kortex::Log
{
	bool IsEnabled();
	bool IsLevelEnabled(Level level);
	void LogString(Level level, const kxf::String& value);
}

namespace Kortex::Log
{
	namespace Private
	{
		inline void LogIfEnabled(Level level, const kxf::String& format)
		{
			if (IsLevelEnabled(level))
			{
				LogString(level, format);
			}
		}
		
		template<class... Args>
		void LogIfEnabled(Level level, const kxf::String& format, Args&&... arg)
		{
			if (IsLevelEnabled(level))
			{
				LogString(level, kxf::String::Format(format, std::forward<Args>(arg)...));
			}
		}
	}

	template<class... Args>
	void Info(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Info, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Error(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Error, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Trace(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Trace, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Debug(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Debug, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Status(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Status, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Message(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Message, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Warning(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Warning, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void Progress(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::Progress, format, std::forward<Args>(arg)...);
	}
	
	template<class... Args>
	void FatalError(const kxf::String& format, Args&&... arg)
	{
		Private::LogIfEnabled(Level::FatalError, format, std::forward<Args>(arg)...);
	}
}
