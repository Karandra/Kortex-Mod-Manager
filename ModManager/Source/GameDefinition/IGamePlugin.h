#pragma once
#include "Framework.hpp"

namespace Kortex
{
	class IGameMod;
}

namespace Kortex
{
	class KORTEX_API IGamePlugin: public kxf::RTTI::Interface<IGamePlugin>
	{
		KxRTTI_DeclareIID(IGamePlugin, {0x977a3069, 0xf8e8, 0x4f7e, {0xb4, 0xd1, 0xe0, 0x94, 0xaa, 0x91, 0x22, 0xbc}});

		public:
			virtual ~IGamePlugin() = default;

		public:
			virtual bool IsNull() const = 0;

			virtual kxf::String GetName() const = 0;
			virtual kxf::FSPath GetPath() const = 0;
			virtual IGameMod* GetOwningMod() const = 0;

			virtual bool IsActive() const = 0;
			virtual void SetActive(bool isActive) = 0;
	
			virtual int GetOrder() const = 0;
			virtual int GetDisplayOrder() const = 0;

			virtual kxf::Enumerator<IGamePlugin&> EnumMasterPlugins() = 0;
			virtual kxf::Enumerator<IGamePlugin&> EnumDependentPlugins() = 0;

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}
	};
}
