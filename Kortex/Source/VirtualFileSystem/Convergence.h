#pragma once
#include "stdafx.h"
#include "FSHandleWrapper.h"
#include "IPC/Common.h"

namespace Kortex::VirtualFileSystem
{
	class Convergence: public FSHandleWrapper
	{
		public:
			Convergence(const wxString& mountPoint, const wxString& writeTarget)
				:FSHandleWrapper(IPC::FileSystemID::Convergence)
			{
				m_Contrller.Send(IPC::RequestID::FSSetMountPoint, IPC::Serializer::Serialize(m_Handle, mountPoint));
				m_Contrller.Send(IPC::RequestID::FSSetWriteTarget, IPC::Serializer::Serialize(m_Handle, writeTarget));
			}

		public:
			void AddVirtualFolder(const wxString& path)
			{
				m_Contrller.Send(IPC::RequestID::FSAddVirtualFolder, IPC::Serializer::Serialize(m_Handle, path));
			}
	
			size_t BuildDispatcherIndex()
			{
				return m_Contrller.Send(IPC::RequestID::FSBuildDispatcherIndex, m_Handle).GetAs<size_t>();
			}
	};
}
