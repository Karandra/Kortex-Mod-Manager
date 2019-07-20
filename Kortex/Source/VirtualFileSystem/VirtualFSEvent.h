#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"

namespace Kortex
{
	class IVirtualFileSystem;
}

namespace Kortex
{
	class VirtualFSEvent: public BroadcastEvent
	{
		public:
			KxEVENT_MEMBER(VirtualFSEvent, SingleToggled);
			KxEVENT_MEMBER(VirtualFSEvent, MainToggled);

		private:
			IVirtualFileSystem* m_FileSustem = nullptr;
			bool m_IsActivated = false;

		public:
			VirtualFSEvent(IVirtualFileSystem& fileSystem, bool activated)
				:m_FileSustem(&fileSystem), m_IsActivated(activated)
			{
			}

		public:
			VirtualFSEvent* Clone() const override
			{
				return new VirtualFSEvent(*this);
			}

			IVirtualFileSystem& GetFileSystem() const
			{
				return *m_FileSustem;
			}
			bool IsActivated() const
			{
				return m_IsActivated;
			}
	};
}
