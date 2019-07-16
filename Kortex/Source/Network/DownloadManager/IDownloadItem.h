#pragma once
#include "stdafx.h"
#include "Network/Common.h"

namespace Kortex
{
	class IDownloadExecutor;
}

namespace Kortex
{
	class IDownloadItem
	{
		public:
			virtual ~IDownloadItem() = default;

		public:
			virtual std::unique_ptr<IDownloadExecutor> OnExecutorDone() = 0;
			virtual void OnUpdateProgress() = 0;
	};
}
