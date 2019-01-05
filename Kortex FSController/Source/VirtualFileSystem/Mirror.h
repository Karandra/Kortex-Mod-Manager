#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxVFSWrapper.h"
#include <KxVirtualFileSystem/AbstractFS.h>
#include <KxVirtualFileSystem/Mirror/MirrorFS.h>

namespace Kortex::VirtualFileSystem
{
	class Mirror: public KxVFSWrapper<KxVFS::MirrorFS>
	{
		public:
			Mirror(const wxString& mountPoint = {}, const wxString& source = {});

		public:
			wxString GetMountPoint() const
			{
				return ToWxString(MirrorFS::GetMountPoint());
			}
			void SetMountPoint(const wxString& mountPoint)
			{
				MirrorFS::SetMountPoint(ToKxDynamicStringRef(mountPoint));
			}

			wxString GetSource() const
			{
				return ToWxString(MirrorFS::GetSource());
			}
			void SetSource(const wxString& source)
			{
				MirrorFS::SetSource(ToKxDynamicStringRef(source));
			}
	};
}