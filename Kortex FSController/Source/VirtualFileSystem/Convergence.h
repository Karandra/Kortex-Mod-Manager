#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxVFSWrapper.h"
#include <KxVirtualFileSystem/IFileSystem.h>
#include <KxVirtualFileSystem/ConvergenceFS.h>

namespace Kortex::VirtualFileSystem
{
	class Convergence: public KxVFSWrapper<KxVFS::ConvergenceFS>
	{
		public:
			Convergence(const wxString& mountPoint = {}, const wxString& writeTarget = {});

		public:
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
