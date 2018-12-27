#pragma once
#include "stdafx.h"
#include "Programs/IProgramEntry.h"
#include "Programs/IProgramManager.h"
#include <KxFramework/KxXML.h>

namespace Kortex::ProgramManager
{
	class DefaultProgramManager: public IProgramManager
	{
		private:
			IProgramEntry::Vector m_DefaultPrograms;
			IProgramEntry::Vector m_UserPrograms;

		private:
			void LoadUserPrograms();
			void SaveUserPrograms() const;

		protected:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

		public:
			const IProgramEntry::Vector& GetProgramList() const override
			{
				return m_UserPrograms;
			}
			IProgramEntry::Vector& GetProgramList() override
			{
				return m_UserPrograms;
			}
			
			std::unique_ptr<IProgramEntry> NewProgram() override;
			void RemoveProgram(IProgramEntry& programEntry) override;

			void LoadDefaultPrograms() override;
	};
}