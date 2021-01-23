#pragma once
#include "Framework.hpp"
#include "IGameDefinition.h"
#include "Application/AppOption.h"
#include "Application/Options/Option.h"

namespace Kortex
{
	class IGameProfile;
}

namespace Kortex
{
	class IGameInstance: public kxf::RTTI::ExtendInterface<IGameInstance, IGameDefinition>, public Application::WithInstanceOptions<IGameInstance>
	{
		KxRTTI_DeclareIID(IGameInstance, {0x6fe63d61, 0x8666, 0x44fc, {0xbb, 0x4e, 0x4f, 0xcd, 0x93, 0x82, 0x82, 0xfc}});

		public:
			enum class CopyFlag: uint32_t
			{
				None = 0,

				Config = 1 << 0
			};

		protected:
			void OnInit()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IGameInstance::OnInit);
			}
			void OnExit()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IGameInstance::OnExit);
			}

		public:
			virtual ~IGameInstance() = default;

		public:
			bool IsActive() const;

			virtual kxf::XMLDocument& GetUserConfig() = 0;
			virtual const kxf::XMLDocument& GetUserConfig() const = 0;

			// Profiles
			virtual IGameProfile* GetActiveProfile() = 0;
			virtual size_t EnumProfiles(std::function<bool(IGameProfile& profile)> func) = 0;
			IGameProfile* GetProfile(const kxf::String& id);

			virtual std::unique_ptr<IGameProfile> CreateProfile(const kxf::String& profileID, const IGameProfile* baseProfile = nullptr, kxf::FlagSet<CopyFlag> copyFlags = {}) = 0;
			virtual bool RemoveProfile(IGameProfile& profile) = 0;
			virtual bool RenameProfile(IGameProfile& profile, const kxf::String& newID) = 0;
			virtual bool SwitchActiveProfile(IGameProfile& profile) = 0;
	};
}

namespace kxf
{
	KxFlagSet_Declare(Kortex::IGameInstance::CopyFlag);
}
