#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Application/IModule.h"
#include "KProgramEntry.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxProcess.h>
class KxMenu;
class KxMenuItem;

class KMainWindow;
class KProgramManagerModel;
class KProgramManagerConfig;

namespace Kortex
{
	namespace ProgramManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class KProgramManager:
		public ManagerWithTypeInfo<IManager, SimpleManagerInfo, ProgramManager::Internal::TypeInfo>,
		public KxSingletonPtr<KProgramManager>
	{
		friend class KMainWindow;
		friend class KProgramManagerModel;
		friend class KProgramManagerConfig;

		private:
			KProgramEntry::Vector m_DefaultPrograms;
			KProgramEntry::Vector m_UserPrograms;

		protected:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

		private:
			void LoadUserPrograms();
			void SaveUserPrograms() const;

			void LoadEntryImages(KProgramEntry& entry) const;
			bool CheckEntryImages(const KProgramEntry& entry) const;
			wxBitmap LoadEntryImage(const KProgramEntry& entry, bool smallBitmap) const;
			void OnAddMainMenuItems(KxMenu& menu);

			KxProcess& DoCreateProcess(const KProgramEntry& entry) const;
			int DoRunProcess(KxProcess& process) const;
			bool DoCheckEntry(const KProgramEntry& entry) const;

		public:
			KProgramManager();
			virtual ~KProgramManager();

		public:
			bool HasPrograms() const
			{
				return !m_UserPrograms.empty();
			}
			const KProgramEntry::Vector& GetProgramList() const
			{
				return m_UserPrograms;
			}
			KProgramEntry::Vector& GetProgramList()
			{
				return m_UserPrograms;
			}

		public:
			virtual void Save() const override;
			virtual void Load() override;
			void LoadDefaultPrograms();

			// Created process can be run with either 'KxPROCESS_WAIT_SYNC' or 'KxPROCESS_WAIT_ASYNC' flag.
			// Or use 'RunProcess' to run it with default parameters.
			// If you didn't run it, call 'DestroyProcess'.
			KxProcess& CreateProcess(const KProgramEntry& entry) const;
			void DestroyProcess(KxProcess& process);
			int RunProcess(KxProcess& process) const;

			// Check entry paths and perform 'CreateProcess -> RunProcess' sequence on it.
			int RunEntry(const KProgramEntry& entry) const;
	};
}

namespace Kortex
{
	namespace Internal
	{
		extern const SimpleModuleInfo ProgramModuleTypeInfo;
	};

	class KProgramModule:
		public ModuleWithTypeInfo<IModule, SimpleModuleInfo, Internal::ProgramModuleTypeInfo>,
		public KxSingletonPtr<KProgramModule>
	{
		private:
			KProgramManager m_ProgramManager;

		protected:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

		public:
			KProgramModule();

		public:
			virtual ManagerRefVector GetManagers() override
			{
				return ToManagersList(m_ProgramManager);
			}
	};
}
