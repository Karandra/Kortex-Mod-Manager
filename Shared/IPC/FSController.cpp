#include "stdafx.h"
#include "FSController.h"
#include "ProcessingWindow.h"

namespace Kortex::IPC
{
	void FSController::OnProcessReady(wxProcessEvent& event)
	{
		// Length is x2 just to not match window that has our name concatenated with something else
		const wxString searchedName = ProcessingWindow::GetWindowName();
		const size_t nameLength = searchedName.length() * 2;

		for (HWND hWnd: m_Process.EnumWindows())
		{
			wxString name;
			::GetWindowTextW(hWnd, wxStringBuffer(name, nameLength), static_cast<int>(nameLength));

			if (name == searchedName)
			{
				Create(hWnd);
				m_IsReady = true;
				break;
			}
		}
	}

	FSController::FSController(const wxString& executablePath)
		:m_Process(executablePath)
	{
		m_Process.Bind(KxEVT_PROCESS_IDLE, &FSController::OnProcessReady, this);
		m_Process.SetOptionEnabled(KxPROCESS_WAIT_INPUT_IDLE);
	}
	FSController::~FSController()
	{
		SendExit();
	}

	void FSController::Run()
	{
		m_Process.Run(KxPROCESS_RUN_SYNC);
	}
	void FSController::SetProcessingWindow(const ProcessingWindow& processingWindow)
	{
		const size_t handle = reinterpret_cast<size_t>(processingWindow.GetHandle());
		m_Process.SetArguments(KxString::Format(wxS("-HWND \"%1\" -PID \"%2\""), handle, KxProcess(0).GetPID()));
	}
}
