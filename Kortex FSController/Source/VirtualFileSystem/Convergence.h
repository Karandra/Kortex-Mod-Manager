#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxVFSWrapper.h"
#include <KxVirtualFileSystem/AbstractFS.h>
#include <KxVirtualFileSystem/Mirror/MirrorFS.h>
#include <KxVirtualFileSystem/Convergence/ConvergenceFS.h>

namespace Kortex::VirtualFileSystem
{
	class Convergence: public KxVFSWrapper<KxVFS::ConvergenceFS>
	{
		public:
			Convergence(const wxString& mountPoint = {}, const wxString& writeTarget = {});

		public:
			wxString GetMountPoint() const
			{
				return ToWxString(ConvergenceFS::GetMountPoint());
			}
			void SetMountPoint(const wxString& mountPoint)
			{
				ConvergenceFS::SetMountPoint(ToKxDynamicStringRef(mountPoint));
			}

			KxVFS::KxDynamicStringRefW GetSource() const = delete;
			void SetSource(KxVFS::KxDynamicStringRefW source) = delete;

			wxString GetWriteTarget() const
			{
				return ToWxString(ConvergenceFS::GetWriteTarget());
			}
			void SetWriteTarget(const wxString& writeTarget)
			{
				return ConvergenceFS::SetWriteTarget(ToKxDynamicStringRef(writeTarget));
			}

			void AddVirtualFolder(const wxString& path)
			{
				ConvergenceFS::AddVirtualFolder(ToKxDynamicStringRef(path));
			}
	};
}
