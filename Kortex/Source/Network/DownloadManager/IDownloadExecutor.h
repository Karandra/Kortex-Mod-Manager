#pragma once
#include "stdafx.h"
#include "IDownloadItem.h"

namespace Kortex
{
	class IDownloadExecutor
	{
		protected:
			std::unique_ptr<IDownloadExecutor> OnTaskEnd(IDownloadItem& item)
			{
				return item.OnExecutorEnd();
			}
			void OnTaskProgress(IDownloadItem& item)
			{
				item.OnExecutorProgress();
			}

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
			virtual wxDateTime GetStartDate() const = 0;

			virtual std::optional<int64_t> RequestContentLength() const = 0;
			virtual wxString GetLocalPath() const = 0;
			virtual wxString GetLocalTempPath() const = 0;
	};
}
