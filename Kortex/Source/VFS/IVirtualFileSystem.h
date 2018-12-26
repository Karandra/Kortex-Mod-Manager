#pragma once
#include "stdafx.h"
class KIPCClient;

namespace Kortex
{
	class IVirtualFileSystem
	{
		friend class KIPCClient;

		protected:
			virtual void OnEnabled() = 0;
			virtual void OnDisabled() = 0;

		public:
			virtual ~IVirtualFileSystem() = default;

		public:
			virtual bool IsEnabled() const = 0;
			virtual void Enable() = 0;
			virtual void Disable() = 0;
	};
}
