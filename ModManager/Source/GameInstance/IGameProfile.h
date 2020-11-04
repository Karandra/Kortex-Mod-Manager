#pragma once
#include "Framework.hpp"
#include "Application/AppOption.h"

namespace Kortex
{
	class IGameMod;
	class IGamePlugin;
	class IGameInstance;
}

namespace Kortex::GameInstance
{
	class ProfileMod final
	{
		private:
			kxf::String m_Signature;
			int m_Priority = -1;
			bool m_IsActive = false;

		public:
			ProfileMod(const IGameMod& mod, bool active);
			ProfileMod(const kxf::String& signature, bool active, int priority = -1);

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

			kxf::String GetSignature() const
			{
				return m_Signature;
			}
			kxf::object_ptr<IGameMod> GetMod() const;

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

namespace Kortex::GameInstance
{
	class ProfilePlugin final
	{
		private:
			kxf::String m_Name;
			int m_Priority = -1;
			bool m_IsActive = false;

		public:
			ProfilePlugin(const IGamePlugin& plugin, bool active);
			ProfilePlugin(const kxf::String& name, bool active, int priority = -1);

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

			const kxf::String& GetName() const
			{
				return m_Name;
			}
			kxf::object_ptr<IGamePlugin> GetPlugin() const;

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

		friend class IGameInstance;
		friend class AppOption;

		public:
			using ProfileMod = GameInstance::ProfileMod;
			using ProfilePlugin = GameInstance::ProfilePlugin;

			enum class Location
			{
				Unknown = -1,

				RootDirectory,
				SavesDirectory,
				ConfigDirectory,
				WriteTargetDirectory,

				ConfigFile
			};
			enum class Option
			{
				Unknown = -1,

				LocalSaves,
				LocalConfig,
			};
			enum class CopyFlag: uint32_t
			{
				None = 0,

				Config = 1 << 0,
				Saves = 1 << 1
			};

		public:
			static bool ValidateID(const kxf::String& id, kxf::String* validID = nullptr);

		public:
			virtual ~IGameProfile() = default;

		public:
			virtual bool IsNull() const = 0;
			bool IsActive() const;

			virtual std::unique_ptr<IGameProfile> Clone() const = 0;
			virtual IGameInstance* GetOwningInstance() const = 0;
			
			virtual kxf::String GetID() const = 0;
			virtual kxf::FSPath GetLocation(Location locationID) const = 0;

			virtual bool IsOptionEnabled(Option option) const = 0;
			virtual bool SetOptionEnabled(Option option, bool enabled = true) = 0;

			virtual void SyncWithCurrentState() = 0;
			virtual size_t EnumMods(std::function<bool(const ProfileMod& gameMod)> func) = 0;
			virtual size_t EnumPlugins(std::function<bool(const ProfilePlugin& gamePlugin)> func) = 0;
	};
}

namespace kxf
{
	KxFlagSet_Declare(Kortex::IGameProfile::CopyFlag);
}
