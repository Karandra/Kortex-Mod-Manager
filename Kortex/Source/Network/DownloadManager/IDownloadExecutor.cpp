#include "stdafx.h"
#include "IDownloadExecutor.h"
#include "IDownloadItem.h" 

namespace Kortex
{
	std::unique_ptr<Kortex::IDownloadExecutor> IDownloadExecutor::OnTaskEnd(IDownloadItem& item)
	{
		return item.OnExecutorEnd();
	}
	void IDownloadExecutor::OnTaskProgress(IDownloadItem& item)
	{
		item.OnExecutorProgress();
	}
}
