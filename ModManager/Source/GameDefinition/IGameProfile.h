#pragma once
#include "Framework.hpp"
#include "Application/AppOption.h"
#include "Application/Options/Option.h"

namespace Kortex
{
	class IGameMod;
	class IGamePlugin;
	class IGameInstance;
}

namespace Kortex
{
	class GameProfileMod final
	{
		private:
			kxf::String m_Signature;
			int m_Priority = -1;
			bool m_IsActive = false;

		public:
			GameProfileMod(const IGameMod& mod, bool active);
			GameProfileMod(kxf::String signature, bool active, int priority = -1);

		public:
			bool IsNull() const noexcept
			{
				return m_Signature.IsEmpty();
			}
			bool IsActive() const
			{
				return m_IsActive;
			}
			int GetPriority() const
			{
				return m_Priority;
			}

			const kxf::String& GetSignature() const&
			{
				return m_Signature;
			}
			kxf::String GetSignature() && noexcept
			{
				return std::move(m_Signature);
			}

			IGameMod* ResolveMod() const;

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

namespace Kortex
{
	class GameProfilePlugin final
	{
		private:
			kxf::String m_Name;
			int m_Priority = -1;
			bool m_IsActive = false;

		public:
			GameProfilePlugin(const IGamePlugin& plugin, bool active);
			GameProfilePlugin(kxf::String name, bool active, int priority = -1);

		public:
			bool IsNull() const
			{
				return m_Name.IsEmpty();
			}
			bool IsActive() const
			{
				return m_IsActive;
			}
			int GetPriority() const
			{
				return m_Priority;
			}

			const kxf::String& GetName() const&
			{
				return m_Name;
			}
			kxf::String GetName() && noexcept
			{
				return std::move(m_Name);
			}

			IGamePlugin* ResolvePlugin() const;

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

namespace Kortex
{
	class IGameProfile: public kxf::RTTI::Interface<IGameProfile>, public Application::WithProfileOptions<IGameProfile>
	{
		KxRTTI_DeclareIID(IGameProfile, {0xb1081844, 0x310c, 0x4773, {0x8c, 0x77, 0xb2, 0x8e, 0xa2, 0x9a, 0x66, 0xd5}});

		public:
			enum class Location
			{
				None = -1,

				Root,
				GameSaves,
				GameConfig,
				WriteTarget,
			};
			enum class Option
			{
				None = -1,

				LocalSaves,
				LocalConfig,
			};

		public:
			static bool ValidateName(const kxf::String& name, kxf::String* validName = nullptr);

		public:
			virtual ~IGameProfile() = default;

		public:
			virtual bool IsNull() const = 0;
			virtual IGameInstance& GetOwningInstance() const = 0;
			bool IsActive() const;

			virtual kxf::XMLDocument& GetProfileData() = 0;
			virtual const kxf::XMLDocument& GetProfileData() const = 0;
			virtual bool LoadProfileData(IGameInstance& owningIntsance, const kxf::IFileSystem& fileSystem) = 0;
			virtual bool SaveProfileData() const = 0;
			
			virtual kxf::String GetName() const = 0;
			virtual kxf::IFileSystem& GetLocation(Location locationID) = 0;

			virtual void SyncWithCurrentState() = 0;
			virtual size_t EnumMods(std::function<bool(const GameProfileMod& gameMod)> func) const = 0;
			virtual size_t EnumPlugins(std::function<bool(const GameProfilePlugin& gamePlugin)> func) const = 0;

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
