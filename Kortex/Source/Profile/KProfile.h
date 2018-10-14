#pragma once
#include "stdafx.h"
#include "KProgramOptions.h"
#include "KEvents.h"
#include <KxFramework/KxSingleton.h>
class KIVariablesTable;
class KModEntry;
class KPluginEntry;

//////////////////////////////////////////////////////////////////////////
class KProfileMod
{
	public:
		using Vector = std::vector<KProfileMod>;

	public:
		wxString m_Signature;
		bool m_IsEnabled = false;

	public:
		KProfileMod(const KModEntry& modEntry, bool enabled);
		KProfileMod(const wxString& signature, bool enabled);

	public:
		bool IsOK() const
		{
			return !m_Signature.IsEmpty();
		}
		
		const wxString& GetSignature() const
		{
			return m_Signature;
		}
		bool IsEnabled() const
		{
			return m_IsEnabled;
		}
		
		KModEntry* GetMod() const;
};

//////////////////////////////////////////////////////////////////////////
class KProfilePlugin
{
	public:
		using Vector = std::vector<KProfilePlugin>;

	public:
		wxString m_PluginName;
		bool m_IsEnabled = false;

	public:
		KProfilePlugin(const KPluginEntry& pluginEntry, bool enabled);
		KProfilePlugin(const wxString& name, bool enabled);

	public:
		bool IsOK() const
		{
			return !m_PluginName.IsEmpty();
		}
		
		const wxString& GetPluginName() const
		{
			return m_PluginName;
		}
		bool IsEnabled() const
		{
			return m_IsEnabled;
		}
		
		KPluginEntry* GetPlugin() const;
};

//////////////////////////////////////////////////////////////////////////
class KProfile
{
	friend class KGameInstance;

	public:
		using Vector = std::vector<std::unique_ptr<KProfile>>;
		using RefVector = std::vector<KProfile*>;
		using CRefVector = std::vector<const KProfile*>;

		static void SetGlobalPaths(KIVariablesTable& variables);
		static wxString ProcessID(const wxString& id);

	private:
		struct FolderNames
		{
			constexpr static const auto Overwrites = wxS("Overwrites");
			constexpr static const auto Saves = wxS("Saves");
			constexpr static const auto Config = wxS("Config");
		};

	private:
		wxString m_ID;
		bool m_LocalSavesEnabled = false;
		bool m_LocalConfigEnabled = false;

		KProfileMod::Vector m_Mods;
		KProfilePlugin::Vector m_Plugins;

	private:
		wxString GetOrderFile() const;
		void SetID(const wxString& id);

	public:
		KProfile(const wxString& id);

	public:
		wxString GetProfileDir() const;
		wxString GetProfileRelativePath(const wxString& name) const;
		wxString GetSavesDir() const;
		wxString GetConfigDir() const;
		wxString GetOverwritesDir() const;

		void Load();
		void Save();
		void SyncWithCurrentState();

		bool IsCurrent() const;
		wxString GetID() const
		{
			return m_ID;
		}

		bool IsLocalSavesEnabled() const
		{
			return m_LocalSavesEnabled;
		}
		void SetLocalSavesEnabled(bool value);

		bool IsLocalConfigEnabled() const
		{
			return m_LocalConfigEnabled;
		}
		void SetLocalConfigEnabled(bool value);

		const KProfileMod::Vector& GetMods() const
		{
			return m_Mods;
		}
		KProfileMod::Vector& GetMods()
		{
			return m_Mods;
		}

		const KProfilePlugin::Vector& GetPlugins() const
		{
			return m_Plugins;
		}
		KProfilePlugin::Vector& GetPlugins()
		{
			return m_Plugins;
		}
};
