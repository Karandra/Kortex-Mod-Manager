#include "stdafx.h"
#include "ProgressOverlay.h"
#include "Application/IMainWindow.h"

namespace
{
	void DoUpdateProgress(int current)
	{
		using namespace Kortex;

		if (IMainWindow* mainWindow = IMainWindow::GetInstance())
		{
			if (wxThread::IsMain())
			{
				mainWindow->SetStatusProgress(current);
			}
			else
			{
				BroadcastProcessor::Get().CallAfter([mainWindow, current]()
				{
					mainWindow->SetStatusProgress(current);
				});
			}
		}
	}
	void DoUpdateProgress(int64_t current, int64_t total)
	{
		using namespace Kortex;

		if (IMainWindow* mainWindow = IMainWindow::GetInstance())
		{
			if (wxThread::IsMain())
			{
				mainWindow->SetStatusProgress(current, total);
			}
			else
			{
				BroadcastProcessor::Get().CallAfter([mainWindow, current, total]()
				{
					mainWindow->SetStatusProgress(current, total);
				});
			}
		}
	}
}

namespace Kortex::UI
{
	ProgressOverlay::ProgressOverlay()
	{
		DoUpdateProgress(0);
	}
	ProgressOverlay::~ProgressOverlay()
	{
		DoUpdateProgress(0);
	}

	bool ProgressOverlay::IsAvailablle() const
	{
		return IMainWindow::GetInstance() != nullptr;
	}

	void ProgressOverlay::UpdateProgress(int current)
	{
		DoUpdateProgress(current);
	}
	void ProgressOverlay::UpdateProgress(int64_t current, int64_t total)
	{
		DoUpdateProgress(current, total);
	}
}
