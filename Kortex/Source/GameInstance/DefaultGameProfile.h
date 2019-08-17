#pragma once
#include "stdafx.h"
#include "IGameProfile.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxXML.h>

namespace Kortex::GameInstance
{
	class DefaultGameProfile: public IGameProfile
	{
		private:
			KxXMLDocument m_Config;

			wxString m_ID;
			bool m_LocalSavesEnabled = false;
			bool m_LocalConfigEnabled = false;

			ProfileMod::Vector m_Mods;
			ProfilePlugin::Vector m_Plugins;

		public:
			DefaultGameProfile() = default;
			DefaultGameProfile(const wxString& id)
				:m_ID(ProcessID(id))
			{
			}

		public:
			const KxXMLDocument& GetConfig() const override
			{
				return m_Config;
			}
			KxXMLDocument& GetConfig() override
			{
				return m_Config;
			}
			void OnConfigChanged(AppOption& option) override;
			void LoadConfig() override;
			void SaveConfig() override;

			std::unique_ptr<IGameProfile> Clone() const override;
			void SyncWithCurrentState() override;

			wxString GetID() const override
			{
				return m_ID;
			}
			void SetID(const wxString& id) override
			{
				m_ID = ProcessID(id);
			}

			bool IsLocalSavesEnabled() const override
			{
				return m_LocalSavesEnabled;
			}
			void SetLocalSavesEnabled(bool value) override;

			bool IsLocalConfigEnabled() const override
			{
				return m_LocalConfigEnabled;
			}
			void SetLocalConfigEnabled(bool value) override;

			const ProfileMod::Vector& GetMods() const override
			{
				return m_Mods;
			}
			ProfileMod::Vector& GetMods() override
			{
				return m_Mods;
			}

			const ProfilePlugin::Vector& GetPlugins() const override
			{
				return m_Plugins;
			}
			ProfilePlugin::Vector& GetPlugins()
			{
				return m_Plugins;
			}
	};
}
