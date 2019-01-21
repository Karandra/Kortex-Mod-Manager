#pragma once
#include "stdafx.h"
#include "DefaultGameInstance.h"

namespace Kortex
{
	class IModule;
	class IManager;

	class IGameProfile;
	class IGameInstance;
	class GameDataModule;
}

namespace Kortex::GameInstance
{
	class InstanceModuleLoader
	{
		private:
			IGameInstance* m_Instance = nullptr;

		protected:
			void LoadGlobalModule(IModule& module, const KxXMLDocument& instanceConfig);
			void LoadModule(IModule& module, const KxXMLNode& node);
			template<class T, bool isAlwaysEnabled = false> std::unique_ptr<T> InitModule(const KxXMLDocument& instanceConfig)
			{
				const KxXMLNode node = instanceConfig.GetFirstChildElement("Definition").GetFirstChildElement(T::GetModuleTypeInfo().GetID());
				if (isAlwaysEnabled || node.GetAttributeBool("Enabled", true))
				{
					auto module = std::make_unique<T>();
					LoadModule(*module, node);
					return module;
				}
				return nullptr;
			}

		protected:
			InstanceModuleLoader(IGameInstance* instance)
				:m_Instance(instance)
			{
			}
	};

	class ActiveGameInstance: public ConfigurableGameInstance, public InstanceModuleLoader
	{
		private:
			KxFileStream m_DirectoryLock;
			wxString m_CurrentProfileID;
		
			std::unique_ptr<Kortex::GameDataModule> m_GameDataModule;

		protected:
			void InitModulesConfig(const KxXMLDocument& instanceConfig);
			void InitVariables(const IGameProfile& profile);
		
			virtual bool OnLoadInstance(const KxXMLDocument& instanceConfig) override;
			virtual bool ShouldInitProfiles() const override;

		public:
			ActiveGameInstance(const IGameInstance& instanceTemplate, const wxString& instanceID);

		public:
			const wxString& GetActiveProfileID() const;
			void SetCurrentProfileID(const wxString& id);

			const IGameProfile* GetActiveProfile() const
			{
				return GetProfile(m_CurrentProfileID);
			}
			IGameProfile* GetActiveProfile()
			{
				return GetProfile(m_CurrentProfileID);
			}

			void DoChangeProfileTo(const IGameProfile& profile);
			void LoadSavedProfileOrDefault();
	};
}
