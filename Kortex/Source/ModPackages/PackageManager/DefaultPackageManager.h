#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include "Application/IWorkspace.h"
#include "IWithScriptExtender.h"

namespace Kortex::PackageDesigner
{
	class WithScriptExtender: public IWithScriptExtender
	{
		private:
			PackageProject::RequirementItem* m_Entry = nullptr;

		public:
			void Assign(PackageProject::RequirementItem& entry)
			{
				m_Entry = &entry;
			}
			const PackageProject::RequirementItem& GetEntry() const override
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
			PackageProject::RequirementGroup m_StandardRequirements;
			WithScriptExtender m_WithScriptExtender;

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;
			void CreateWorkspaces() override;

		public:
			IWorkspace::RefVector EnumWorkspaces() const override;

			const PackageProject::RequirementItem::Vector& GetStdRequirements() const override
			{
				return m_StandardRequirements.GetItems();
			}
			const PackageProject::RequirementItem* FindStdReqirement(const wxString& id) const override
			{
				return m_StandardRequirements.FindItem(id);
			}

			void OnModListMenu(KxMenu& menu, const std::vector<IGameMod*>& selectedMods, IGameMod* focusedMod) override;
	};
}
