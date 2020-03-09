#include "stdafx.h"
#include "FSController.h"
#include "ProcessingWindow.h"
#include <thread>

namespace Kortex::IPC
{
	void FSController::OnProcessReady(wxProcessEvent& event)
	{
		// Length is x2 just to not match a window that has our name concatenated with something else
		const wxString searchedName = ProcessingWindow::GetWindowName();
		const size_t nameLength = searchedName.length() * 2;

		for (HWND hWnd: m_Process.EnumWindows())
		{
			wxString name;
			::GetWindowTextW(hWnd, wxStringBuffer(name, nameLength), static_cast<int>(nameLength));

			if (name == searchedName)
			{
				Create(hWnd);
				m_IsProcessIdle = true;
				break;
			}
		}
	}
	void FSController::EndWaitForTermination()
	{
		if (m_ProcessHandle)
		{
			if (std::unique_lock lock(m_ThreadMutex); true)
			{
				Reset();
				SendExit();

				m_ThreadCondition.wait(lock);
			}
		}
	}
	bool FSController::Reset()
	{
		m_IsProcessIdle = false;

		if (m_ProcessHandle)
		{
			::CloseHandle(m_ProcessHandle);
			m_ProcessHandle = nullptr;

			return true;
		}
		return false;
	}
	
	void FSController::OnSendMessage(const Message& message, const void* userData, size_t dataSize)
	{
		if (message.GetRequestID() == RequestID::Exit)
		{
			EndWaitForTermination();
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
		m_IsDestroyed = true;
		EndWaitForTermination();
	}

	bool FSController::IsRunning() const
	{
		return m_ProcessHandle != nullptr || m_IsProcessIdle;
	}

	void FSController::SetProcessingWindow(const ProcessingWindow& processingWindow)
	{
		const size_t handle = reinterpret_cast<size_t>(processingWindow.GetHandle());
		m_Process.SetArguments(KxString::Format(wxS("-HWND \"%1\" -PID \"%2\" -Library \"%3\""), handle, KxProcess(0).GetPID(), m_Library));
	}
	bool FSController::WaitForTermination(std::function<void()> func)
	{
		const uint32_t pid = m_Process.GetPID();
		if (pid != 0 && m_ProcessHandle == nullptr)
		{
			std::thread([this, pid, func = std::move(func)]()
			{
				const HANDLE handle = ::OpenProcess(SYNCHRONIZE, false, pid);
				if (std::unique_lock lock(m_ThreadMutex); true)
				{
					m_ProcessHandle = handle;
				}

				if (handle)
				{
					const DWORD status = ::WaitForSingleObject(handle, INFINITE);
					if (std::unique_lock lock(m_ThreadMutex); true)
					{
						Reset();
						if (status == WAIT_OBJECT_0 && func && !m_IsDestroyed)
						{
							func();
						}
					}
					m_ThreadCondition.notify_one();
				}
			}).detach();
			return true;
		}
		return false;
	}
	void FSController::Run()
	{
		m_Process.Run(KxPROCESS_RUN_SYNC);
	}
}
