#pragma once
#include "Framework.hpp"
#include "GameID.h"
#include "Application/AppOption.h"
#include "Application/Options/Option.h"
#include <kxf/RTTI/QueryInterface.h>
#include <kxf/General/IVariablesCollection.h>

namespace Kortex
{
	class IGameProfile;
}

namespace Kortex
{
	class IGameInstance: public kxf::RTTI::Interface<IGameInstance>, public Application::WithInstanceOptions<IGameInstance>
	{
		KxRTTI_DeclareIID(IGameInstance, {0x6fe63d61, 0x8666, 0x44fc, {0xbb, 0x4e, 0x4f, 0xcd, 0x93, 0x82, 0x82, 0xfc}});

		public:
			enum class Location
			{
				Unknown = -1,

				RootDirectory,
				ProfilesDirectory,
				ModsDirectory,
				GameDirectory,

				IconFile,
				ConfigFile,
			};
			enum class CopyFlag: uint32_t
			{
				None = 0,

				Config = 1 << 0
			};

		public:
			static bool ValidateID(const kxf::String& id, kxf::String* validID = nullptr);

			static wxBitmap GetGenericIcon();
			static kxf::FSPath GetGenericIconLocation();

		protected:
			wxBitmap LoadIcon(const kxf::String& path) const;
			kxf::FSPath GetDefaultIconLocation() const;

		public:
			virtual ~IGameInstance() = default;

		public:
			virtual bool IsNull() const = 0;
			bool IsActive() const;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;
			virtual kxf::String ExpandVariablesLocally(const kxf::String& variables) const = 0;

			virtual int GetSortOrder() const = 0;
			virtual GameID GetGameID() const = 0;
			virtual kxf::String GetID() const = 0;
			virtual kxf::String GetGameName() const = 0;
			virtual kxf::String GetGameShortName() const = 0;
			virtual kxf::FSPath GetLocation(Location locationID) const = 0;
			virtual wxBitmap GetIcon(const kxf::Size& iconSize = kxf::Size::UnspecifiedSize()) const = 0;

			// Profiles
			virtual IGameProfile* GetActiveProfile() = 0;
			virtual size_t EnumProfiles(std::function<bool(IGameProfile& profile)> func) = 0;
			IGameProfile* GetProfile(const kxf::String& id);

			virtual std::unique_ptr<IGameProfile> CreateProfile(const kxf::String& profileID, const IGameProfile* baseProfile = nullptr, kxf::FlagSet<CopyFlag> copyFlags = {}) = 0;
			virtual bool RemoveProfile(IGameProfile& profile) = 0;
			virtual bool RenameProfile(IGameProfile& profile, const kxf::String& newID) = 0;
			virtual bool SwitchActiveProfile(IGameProfile& profile) = 0;

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
	class IConfigurableGameInstance: public kxf::RTTI::ExtendInterface<IConfigurableGameInstance, Application::IWithConfig>
	{
		KxRTTI_DeclareIID(IConfigurableGameInstance, {0x2ef792f5, 0x69e7, 0x4f81, {0xb1, 0xbc, 0xfd, 0xfb, 0xc5, 0x4c, 0xe0, 0xda}});

		public:
			virtual void OnExit() = 0;
	};
}

namespace kxf
{
	KxFlagSet_Declare(Kortex::IGameInstance::CopyFlag);
}
