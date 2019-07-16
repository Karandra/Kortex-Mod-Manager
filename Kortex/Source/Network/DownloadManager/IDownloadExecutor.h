#pragma once
#include "stdafx.h"

namespace Kortex
{
	class DownloadItem;
}
namespace Kortex
{
	class IDownloadExecutor
	{
		public:
			virtual ~IDownloadExecutor() = default;

		public:
			virtual bool IsRunning() const = 0;
			virtual bool IsPaused() const = 0;
			virtual bool IsFailed() const = 0;
			virtual bool IsCompleted() const = 0;

			virtual void Stop() = 0;
			virtual bool Pause() = 0;
			virtual bool Resume() = 0;
			virtual bool Start(int64_t startAt = 0) = 0;

			virtual int64_t GetSpeed() const = 0;
			virtual int64_t GetTotalSize() const = 0;
			virtual int64_t GetDownloadedSize() const = 0;
			virtual int64_t RequestContentLength() const = 0;
			virtual wxDateTime GetStartDate() const = 0;
	};
}
