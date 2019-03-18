#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxVFSWrapper.h"
#include <KxVirtualFileSystem/MirrorFS.h>

namespace Kortex::VirtualFileSystem
{
	class Mirror: public KxVFSWrapper<KxVFS::MirrorFS>
	{
		public:
			Mirror(const wxString& mountPoint = {}, const wxString& source = {});

		public:
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