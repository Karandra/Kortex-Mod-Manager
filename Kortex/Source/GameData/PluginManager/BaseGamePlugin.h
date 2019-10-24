#pragma once
#include "stdafx.h"
#include "GameData/IGamePlugin.h"
#include <KxFramework/KxFileItem.h>

namespace Kortex::PluginManager
{
	class BaseGamePlugin: public IGamePlugin
	{
		private:
			KxFileItem m_FileItem;
			mutable const StdContentEntry* m_StdContent = nullptr;
			bool m_IsActive = false;

		protected:
			void Create(const wxString& fullPath)
			{
				m_FileItem.SetFullPath(fullPath);
				m_FileItem.UpdateInfo();
			}

		public:
			BaseGamePlugin() = default;
			BaseGamePlugin(const wxString& fullPath)
				:m_FileItem(fullPath)
			{
			}

		public:
			bool IsOK() const override
			{
				return m_FileItem.IsOK();
			}

			wxString GetName() const override
			{
				return m_FileItem.GetName();
			}
			wxString GetFullPath() const override
			{
				return m_FileItem.GetFullPath();
			}

			bool IsActive() const override
			{
				return m_IsActive;
			}
			void SetActive(bool isActive) override
			{
				m_IsActive = isActive;
			}
			
			const StdContentEntry* GetStdContentEntry() const override;
	};
}
