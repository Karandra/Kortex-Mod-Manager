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
	class KORTEX_API IGameInstance: public kxf::RTTI::Interface<IGameInstance>, public Application::WithInstanceOptions<IGameInstance>
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

			virtual kxf::XMLDocument& GetnstanceData() = 0;
			virtual const kxf::XMLDocument& GetnstanceData() const = 0;
			virtual bool LoadInstanceData(const kxf::IFileSystem& fileSystem) = 0;
			virtual bool SaveInstanceData() = 0;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual const kxf::IVariablesCollection& GetVariables() const = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;

			virtual kxf::IFileSystem& GetFileSystem(Location locationID) = 0;
			virtual const kxf::IFileSystem& GetFileSystem(Location locationID) const = 0;

		public:
			virtual IGameProfile* GetActiveProfile() const = 0;
			virtual size_t EnumProfiles(std::function<bool(IGameProfile& profile)> func) = 0;
			IGameProfile* GetProfileByName(const kxf::String& profileName);

			virtual IGameProfile* CreateProfile(const kxf::String& profileName, const IGameProfile* baseProfile = nullptr) = 0;
			virtual bool RemoveProfile(IGameProfile& profile) = 0;
			virtual bool RenameProfile(IGameProfile& profile, const kxf::String& newName) = 0;
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
