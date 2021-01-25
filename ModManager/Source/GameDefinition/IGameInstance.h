#pragma once
#include "Framework.hpp"
#include "Application/AppOption.h"
#include "Application/Options/Option.h"

namespace kxf
{
	class XMLDocument;
}
namespace Kortex
{
	class IGameProfile;
	class IGameDefinition;
}

namespace Kortex
{
	class IGameInstance: public kxf::RTTI::Interface<IGameInstance>, public Application::WithInstanceOptions<IGameInstance>
	{
		KxRTTI_DeclareIID(IGameInstance, {0x6fe63d61, 0x8666, 0x44fc, {0xbb, 0x4e, 0x4f, 0xcd, 0x93, 0x82, 0x82, 0xfc}});

		public:
			enum class Location
			{
				Root,
				Game,
				Mods,
				Profiles,
				Downloads,
				MountedGame
			};
			enum class CopyFlag: uint32_t
			{
				None = 0,

				Config = 1 << 0
			};

		public:
			static bool ValidateName(const kxf::String& name, kxf::String* validName = nullptr);

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

			virtual bool IsNull() const = 0;
			virtual IGameDefinition& GetDefinition() const = 0;
			virtual kxf::String GetName() const = 0;

			virtual kxf::XMLDocument& GetUserConfig() = 0;
			virtual const kxf::XMLDocument& GetUserConfig() const = 0;
			virtual bool LoadInstanceData(const kxf::IFileSystem& fileSystem) = 0;
			virtual bool SaveInstanceData() = 0;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual const kxf::IVariablesCollection& GetVariables() const = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;

			virtual kxf::IFileSystem& GetFileSystem(Location locationID) = 0;
			const kxf::IFileSystem& GetFileSystem(Location locationID) const
			{
				return const_cast<IGameInstance&>(*this).GetFileSystem(locationID);
			}

		public:
			virtual IGameProfile* GetActiveProfile() const = 0;
			virtual size_t EnumProfiles(std::function<bool(IGameProfile& profile)> func) = 0;
			IGameProfile* GetProfile(const kxf::String& profileName);

			virtual std::unique_ptr<IGameProfile> CreateProfile(const kxf::String& profileName, const IGameProfile* baseProfile = nullptr, kxf::FlagSet<CopyFlag> copyFlags = {}) = 0;
			virtual bool RemoveProfile(IGameProfile& profile) = 0;
			virtual bool RenameProfile(IGameProfile& profile, const kxf::String& newName) = 0;
			virtual bool SwitchActiveProfile(IGameProfile& profile) = 0;
	};
}

namespace kxf
{
	KxFlagSet_Declare(Kortex::IGameInstance::CopyFlag);
}
