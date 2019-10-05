#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include "IWithScriptExtender.h"

namespace Kortex::PackageDesigner
{
	class WithScriptExtender: public IWithScriptExtender
	{
		private:
			KPPRRequirementEntry* m_Entry = nullptr;

		public:
			void Assign(KPPRRequirementEntry& entry)
			{
				m_Entry = &entry;
			}
			const KPPRRequirementEntry& GetEntry() const override
			{
				return *m_Entry;
			}
	};
}

namespace Kortex::PackageDesigner
{
	class DefaultPackageManager: public IPackageManager
	{
		private:
			KPPRRequirementsGroup m_StandardRequirements;
			WithScriptExtender m_WithScriptExtender;

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;

		public:
			const KPPRRequirementEntry::Vector& GetStdRequirements() const override
			{
				return m_StandardRequirements.GetEntries();
			}
			const KPPRRequirementEntry* FindStdReqirement(const wxString& id) const override
			{
				return m_StandardRequirements.FindEntry(id);
			}

			void OnModListMenu(KxMenu& menu, const std::vector<IGameMod*>& selectedMods, IGameMod* focusedMod) override;
	};
}
