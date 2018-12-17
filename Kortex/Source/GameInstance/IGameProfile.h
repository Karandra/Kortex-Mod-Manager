#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class IVariableTable;

namespace Kortex
{
	class IGameMod;
	class IGamePlugin;
	class IGameInstance;

	namespace GameInstance
	{
		namespace FolderName
		{
			constexpr static const auto Overwrites = wxS("Overwrites");
			constexpr static const auto Saves = wxS("Saves");
			constexpr static const auto Config = wxS("Config");
		}
	}
}

namespace Kortex::GameInstance
{
	class ProfileMod
	{
		public:
			using Vector = std::vector<ProfileMod>;

		public:
			wxString m_Signature;
			bool m_IsActive = false;

		public:
			ProfileMod(const IGameMod& modEntry, bool active);
			ProfileMod(const wxString& signature, bool active);

		public:
			bool IsOK() const
			{
				return !m_Signature.IsEmpty();
			}

			const wxString& GetSignature() const
			{
				return m_Signature;
			}
			bool IsActive() const
			{
				return m_IsActive;
			}
		
			IGameMod* GetMod() const;
	};
}

namespace Kortex::GameInstance
{
	class ProfilePlugin
	{
		public:
			using Vector = std::vector<ProfilePlugin>;

		public:
			wxString m_PluginName;
			bool m_IsActive = false;

		public:
			ProfilePlugin(const IGamePlugin& plugin, bool active);
			ProfilePlugin(const wxString& name, bool active);

		public:
			bool IsOK() const
			{
				return !m_PluginName.IsEmpty();
			}
		
			bool IsActive() const
			{
				return m_IsActive;
			}
			const wxString& GetPluginName() const
			{
				return m_PluginName;
			}
			IGamePlugin* GetPlugin() const;
	};
}

namespace Kortex
{
	class IGameProfile
	{
		friend class IGameInstance;

		public:
			using Vector = std::vector<std::unique_ptr<IGameProfile>>;
			using RefVector = std::vector<IGameProfile*>;
			using CRefVector = std::vector<const IGameProfile*>;

		public:
			static IGameProfile* GetActive();
			static wxString ProcessID(const wxString& id);
			static void SetGlobalPaths(IVariableTable& variables);

		protected:
			static bool CreateLocalFolder(const wxString& id, const wxString& name);

		public:
			virtual ~IGameProfile() = default;

		public:
			bool IsActive() const
			{
				return GetActive() == this;
			}

			wxString GetOrderFile() const;
			wxString GetProfileDir() const;
			wxString GetProfileRelativePath(const wxString& name) const;
			wxString GetSavesDir() const;
			wxString GetConfigDir() const;
			wxString GetOverwritesDir() const;

			virtual std::unique_ptr<IGameProfile> Clone() const = 0;
			virtual void Load() = 0;
			virtual void Save() = 0;
			virtual void SyncWithCurrentState() = 0;

			virtual wxString GetID() const = 0;
			virtual void SetID(const wxString& id) = 0;

			virtual bool IsLocalSavesEnabled() const = 0;
			virtual void SetLocalSavesEnabled(bool value) = 0;

			virtual bool IsLocalConfigEnabled() const = 0;
			virtual void SetLocalConfigEnabled(bool value) = 0;

			virtual const GameInstance::ProfileMod::Vector& GetMods() const = 0;
			virtual GameInstance::ProfileMod::Vector& GetMods() = 0;

			virtual const GameInstance::ProfilePlugin::Vector& GetPlugins() const = 0;
			virtual GameInstance::ProfilePlugin::Vector& GetPlugins() = 0;
	};
}
