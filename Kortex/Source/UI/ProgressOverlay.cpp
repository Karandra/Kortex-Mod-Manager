#include "stdafx.h"
#include "ProgressOverlay.h"
#include "Application/IMainWindow.h"

namespace Kortex::UI
{
	ProgressOverlay::ProgressOverlay()
	{
		UpdateProgress(0);
	}
	ProgressOverlay::~ProgressOverlay()
	{
		UpdateProgress(0);
	}

	bool ProgressOverlay::IsAvailablle() const
	{
		return IMainWindow::GetInstance() != nullptr;
	}

	void ProgressOverlay::UpdateProgress(int current)
	{
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
	void ProgressOverlay::UpdateProgress(int64_t current, int64_t total)
	{
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
