#pragma once
#include "stdafx.h"
#include "AbstractFS.h"

namespace Kortex::VirtualFileSystem
{
	class Mirror: public AbstractFS
	{
		protected:
			Mirror(IPC::FileSystemID id, const wxString& mountPoint, const wxString& source);

		public:
			Mirror(const wxString& mountPoint, const wxString& source);

		public:
			void SetSource(const wxString& path);
	};
}
