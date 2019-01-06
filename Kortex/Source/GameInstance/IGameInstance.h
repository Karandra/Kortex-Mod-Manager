#pragma once
#include "stdafx.h"
#include "GameID.h"
#include "Application/IAppOption.h"
#include "Application/Options/Option.h"
#include <KxFramework/KxINI.h>
#include <KxFramework/KxFileStream.h>
class IVariableTable;

namespace Kortex
{
	namespace GameInstance
	{
		class TemplateLoader;

		enum CopyOptionsInstance: uint32_t
		{
			Config = 1 << 0,
		};
		enum CopyOptionsProfile: uint32_t
		{
			GameConfig = 1 << 0,
			GameSaves = 1 << 1,
		};
	}

	class IGameProfile;
	class IGameInstance:
		public RTTI::IInterface<IGameInstance>,
		public Application::WithInstanceOptions<IGameInstance>
	{
		friend class GameInstance::TemplateLoader;

		public:
			using Vector = std::vector<std::unique_ptr<IGameInstance>>;
			using RefVector = std::vector<IGameInstance*>;
			using CRefVector = std::vector<const IGameInstance*>;
			using ProfilesVector = std::vector<std::unique_ptr<IGameProfile>>;

		public:
			static bool IsValidInstanceID(const wxString& id);
			static bool IsValidProfileID(const wxString& id);

			static wxBitmap GetGenericIcon();
			static wxString GetGenericIconLocation();

		public:
			static IGameInstance* CreateActive(const IGameInstance& instanceTemplate, const wxString& instanceID);
			static IGameInstance* GetActive();
			static IGameInstance& AssignActive(std::unique_ptr<IGameInstance> instance);
			static void DestroyActive();

			static Vector& GetShallowInstances();
			static IGameInstance* GetShallowInstance(const wxString& instanceID);
			static IGameInstance* NewShallowInstance(const wxString& instanceID);

			static wxString GetTemplatesFolder();
			static wxString GetUserTemplatesFolder();
			static void LoadTemplates();
			static void LoadInstances();

			static size_t GetTemplatesCount();
			static Vector& GetTemplates();
			static IGameInstance* GetTemplate(const GameID& id);
			static bool HasTemplate(const GameID& id);

		public:
			static wxString GetActiveProfileID();
			static IGameProfile* GetActiveProfile();
			static bool IsActiveProfileID(const wxString& id);

		protected:
			wxBitmap LoadIcon(const wxString& path) const;
			wxString GetDefaultIconLocation() const;

			virtual wxString CreateProfileID(const wxString& id) const = 0;
			virtual wxString CreateDefaultProfileID() const = 0;

			virtual bool ShouldInitProfiles() const = 0;
			virtual bool InitInstance() = 0;

		public:
			virtual ~IGameInstance() = default;

		public:
			virtual bool IsOK() const = 0;
			virtual bool IsTemplate() const = 0;
			virtual wxString GetDefinitionFile() const = 0;
		
			// Variables
			virtual IVariableTable& GetVariables() = 0;
			virtual const IVariableTable& GetVariables() const = 0;
			virtual wxString ExpandVariablesLocally(const wxString& variables) const = 0;
			virtual wxString ExpandVariables(const wxString& variables) const = 0;

			// Properties
			virtual GameID GetGameID() const = 0;
			virtual wxString GetInstanceID() const = 0;
			virtual wxString GetGameName() const = 0;
			virtual wxString GetGameShortName() const = 0;
		
			virtual int GetSortOrder() const = 0;
			virtual bool IsSystemTemplate() const = 0;
			virtual wxString GetIconLocation() const = 0;
			virtual wxBitmap GetIcon() const = 0;
			bool IsActiveInstance() const;

			virtual wxString GetInstanceTemplateDir() const = 0;
			virtual wxString GetInstanceDir() const = 0;
			virtual wxString GetInstanceRelativePath(const wxString& name) const = 0;

			virtual wxString GetConfigFile() const = 0;
			virtual wxString GetModsDir() const = 0;
			virtual wxString GetProfilesDir() const = 0;
			virtual wxString GetGameDir() const = 0;
			virtual wxString GetVirtualGameDir() const = 0;

			// Instances
			const IGameInstance& GetTemplate() const;
			IGameInstance& GetTemplate();

			bool Deploy(const IGameInstance* baseInstance = nullptr, uint32_t copyOptions = 0);
			bool WithdrawDeploy();
			bool IsDeployed() const;

			// Profiles
			bool HasProfiles() const
			{
				return !GetProfiles().empty();
			}
			size_t GetProfilesCount() const
			{
				return GetProfiles().size();
			}
			virtual const ProfilesVector& GetProfiles() const = 0;
			virtual ProfilesVector& GetProfiles() = 0;

			bool HasProfile(const wxString& id) const
			{
				return GetProfile(id) != nullptr;
			}
			virtual const IGameProfile* GetProfile(const wxString& id) const = 0;
			virtual IGameProfile* GetProfile(const wxString& id) = 0;

			virtual std::unique_ptr<IGameProfile> NewProfile() = 0;
			virtual IGameProfile* CreateProfile(const wxString& profileID, const IGameProfile* baseProfile = nullptr, uint32_t copyOptions = 0) = 0;
			virtual IGameProfile* ShallowCopyProfile(const IGameProfile& profile, const wxString& nameSuggets = wxEmptyString) = 0;
			virtual bool RemoveProfile(IGameProfile& profile) = 0;
			virtual bool RenameProfile(IGameProfile& profile, const wxString& newID) = 0;
			virtual bool ChangeProfileTo(const IGameProfile& profile) = 0;
			virtual void LoadSavedProfileOrDefault() = 0;
	};
}

namespace Kortex
{
	class IConfigurableGameInstance:
		public RTTI::IInterface<IConfigurableGameInstance>,
		public Application::IWithConfig
	{
		public:
			virtual void OnExit() = 0;
	};
}
