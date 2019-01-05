#include "stdafx.h"
#include "Convergence.h"
#include "IPC/Common.h"

using namespace Kortex::IPC;

namespace Kortex::VirtualFileSystem
{
	Convergence::Convergence(FileSystemID id, const wxString& mountPoint, const wxString& writeTarget)
		:Mirror(id, mountPoint, writeTarget)
	{
	}
	Convergence::Convergence(const wxString& mountPoint, const wxString& writeTarget)
		:Convergence(FileSystemID::Convergence, mountPoint, writeTarget)
	{
	}

	void Convergence::AddVirtualFolder(const wxString& path)
	{
		m_Contrller.Send(RequestID::FSAddVirtualFolder, m_Handle, path);
	}
	size_t Convergence::BuildDispatcherIndex()
	{
		return m_Contrller.Send(RequestID::FSBuildDispatcherIndex, m_Handle).GetAs<size_t>();
	}

	bool Convergence::EnableINIOptimization(bool value)
	{
		return m_Contrller.Send(RequestID::FSEnableINIOptimization, m_Handle).GetAs<bool>();
	}
	bool Convergence::EnableSecurityFunctions(bool value)
	{
		return m_Contrller.Send(RequestID::FSEnableSecurityFunctions, m_Handle).GetAs<bool>();
	}
}
