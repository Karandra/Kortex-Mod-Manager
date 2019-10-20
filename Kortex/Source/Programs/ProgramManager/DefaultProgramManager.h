#pragma once
#include "stdafx.h"
#include "Programs/IProgramItem.h"
#include "Programs/IProgramManager.h"
#include "Application/IWorkspace.h"
#include <KxFramework/KxXML.h>

namespace Kortex::ProgramManager
{
	class DefaultProgramManager: public IProgramManager
	{
		private:
			IProgramItem::Vector m_DefaultPrograms;
			IProgramItem::Vector m_UserPrograms;

		private:
			void LoadUserPrograms();
			void SaveUserPrograms() const;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void CreateWorkspaces() override;

		public:
			IWorkspace::RefVector EnumWorkspaces() const override;

			const IProgramItem::Vector& GetProgramList() const override
			{
				return m_UserPrograms;
			}
			IProgramItem::Vector& GetProgramList() override
			{
				return m_UserPrograms;
			}
			
			std::unique_ptr<IProgramItem> NewProgram() override;
			void RemoveProgram(IProgramItem& programEntry) override;
			void LoadDefaultPrograms() override;
	};
}
