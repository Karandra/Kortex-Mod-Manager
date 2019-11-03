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
			enum class Error
			{
				Success,
				Unknown,
				NonEmptyMountPoint
			};

		public:
			KxEVENT_MEMBER(VirtualFSEvent, SingleToggled);
			KxEVENT_MEMBER(VirtualFSEvent, MainToggled);
			KxEVENT_MEMBER(VirtualFSEvent, MainToggleError);

		private:
			IVirtualFileSystem* m_FileSustem = nullptr;
			Error m_Error = Error::Success;
			bool m_IsActivated = false;

			KxStringVector m_MountPoints;

		public:
			VirtualFSEvent(IVirtualFileSystem& fileSystem, bool activated, Error error = Error::Success)
				:m_FileSustem(&fileSystem), m_IsActivated(activated), m_Error(error)
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
			
			Error GetErrorType() const
			{
				return m_Error;
			}
			void SetError(Error error)
			{
				m_Error = error;
			}

			KxStringVector GetMountPoints()
			{
				return std::move(m_MountPoints);
			}
			void SetMountPoints(KxStringVector items)
			{
				m_MountPoints = std::move(items);
			}
	};
}
