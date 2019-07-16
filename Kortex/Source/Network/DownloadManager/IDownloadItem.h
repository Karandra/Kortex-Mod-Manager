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
		friend class IDownloadExecutor;

		protected:
			virtual std::unique_ptr<IDownloadExecutor> OnExecutorEnd() = 0;
			virtual void OnExecutorProgress() = 0;

		public:
			virtual ~IDownloadItem() = default;
	};
}
