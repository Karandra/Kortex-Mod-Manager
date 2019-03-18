#pragma once
#include "stdafx.h"
#include "BaseFileSystem.h"

namespace Kortex::VirtualFileSystem
{
	class Mirror: public BaseFileSystem
	{
		protected:
			Mirror(IPC::FileSystemID id, const wxString& mountPoint, const wxString& source);

		public:
			Mirror(const wxString& mountPoint, const wxString& source);

		public:
			void SetSource(const wxString& path);
	};
}
