#pragma once
#include "stdafx.h"
#include "VFS/IVirtualFileSystem.h"
class KxProgressDialog;

namespace Kortex::ModManager
{
	class DefaultModManager;

	class VirtualFileSystem: public IVirtualFileSystem
	{
		private:
			DefaultModManager& m_Manager;
			KxProgressDialog* m_StatusDialog = nullptr;
			bool m_IsEnabled = false;

		private:
			KxStringVector CheckMountPoints() const;
			void InitMainVirtualFolder();
			void InitMirroredLocations();

			void ShowStatusDialog();
			void HideStatusDialog();

		protected:
			void OnEnabled();
			void OnDisabled();

		public:
			VirtualFileSystem(DefaultModManager& manager)
				:m_Manager(manager)
			{
			}

		public:
			bool IsEnabled() const override;
			void Enable() override;
			void Disable() override;
	};
}
