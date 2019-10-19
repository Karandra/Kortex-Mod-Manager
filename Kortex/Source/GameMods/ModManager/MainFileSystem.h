#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
class KxProcess;
class KxProgressDialog;

namespace Kortex
{
	class VirtualFSEvent;
}
namespace Kortex::VirtualFileSystem
{
	class BaseFileSystem;
}

namespace Kortex::ModManager
{
	class DefaultModManager;

	class MainFileSystem: public IVirtualFileSystem
	{
		private:
			using BaseFileSystem = VirtualFileSystem::BaseFileSystem;
			using ProcessList = std::vector<std::unique_ptr<KxProcess>>;

		private:
			DefaultModManager& m_Manager;
			BroadcastReciever m_BroadcastReciever;
			KxProgressDialog* m_StatusDialog = nullptr;
			
			size_t m_InstancesCountEnabled = 0;
			size_t m_InstancesCountTotal = 0;
			bool m_IsEnabled = false;

			std::unique_ptr<BaseFileSystem> m_Convergence;
			std::vector<std::unique_ptr<BaseFileSystem>> m_Mirrors;

		private:
			KxStringVector CheckMountPoints() const;
			void InitMainVirtualFolder();
			void InitMirroredLocations();
			void SetFileSystemOptions(VirtualFileSystem::BaseFileSystem& fileSystem);
			
			ProcessList CheckAllForRunningPrograms();
			void CheckForRunningPrograms(ProcessList& activeProcesses, const KxUInt32Vector& processes, BaseFileSystem& fileSystem);

			void ShowStatusDialog();
			void HideStatusDialog();

			template<class TFunctor> BaseFileSystem* ForEachInstance(TFunctor&& func) const
			{
				if (func(m_Convergence.get()))
				{
					return m_Convergence.get();
				}

				for (auto& vfs: m_Mirrors)
				{
					if (func(vfs.get()))
					{
						return vfs.get();
					}
				}
				return nullptr;
			}
			size_t GetInstancesCount() const;
			
			bool IsOurInstance(const IVirtualFileSystem& instance) const;
			void OnSingleFSMounted(VirtualFSEvent& event);
			void OnSingleFSUnmounted(VirtualFSEvent& event);

		protected:
			void OnEnabled() override;
			void OnDisabled() override;

		public:
			MainFileSystem(DefaultModManager& manager);
			virtual ~MainFileSystem();

		public:
			IPC::FSHandle GetHandle() const override
			{
				return 0;
			}

			bool IsEnabled() const override;
			void Enable() override;
			void Disable() override;
	};
}
