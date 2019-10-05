#pragma once
#include "stdafx.h"
#include "DisplayModelNode.h"
#include "Utility/KBitmapSize.h"
#include "Utility/KAux.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex
{
	class IGameMod;
	class IModTag;
}

namespace Kortex::ModManager
{
	enum class DisplayModelType
	{
		Connector,
		Manager,
	};

	class DisplayModel: public KxRTTI::ExtendInterface<DisplayModel, KxDataView2::Model>
	{
		friend class Workspace;
		friend class DisplayModelModNode;

		public:
			using ColumnID = DisplayModelColumnID;
			enum class PriorityGroupLabelAlignment
			{
				Center,
				Left,
				Right,
			};

		public:
			static wxString FormatTagList(const IGameMod& entry);

		private:
			bool m_ShowPriorityGroups = false;
		
			DisplayModelType m_DisplayMode = DisplayModelType::Connector;
			bool m_ShowNotInstalledMods = false;
			bool m_BoldPriorityGroupLabels = false;

			PriorityGroupLabelAlignment m_PriorityGroupLabelAlignmentType = PriorityGroupLabelAlignment::Center;
			int m_PriorityGroupLabelAlignment = wxALIGN_INVALID;
			int m_PriorityGroupRowHeight = 0;
			KxColor m_PriortyGroupColor;

			KxDataView2::Column* m_NameColumn = nullptr;
			KxDataView2::Column* m_PriorityColumn = nullptr;

			std::unique_ptr<IModTag> m_NoneTag;
			std::vector<DisplayModelModNode> m_ModNodes;
			std::unordered_map<wxString, DisplayModelTagNode> m_TagNodes;
			std::vector<std::unique_ptr<PriorityGroup>> m_PriortyGroups;
		
			wxString m_SearchMask;
			std::vector<KxDataView2::Column*> m_SearchColumns;

		private:
			void CreateView(wxWindow* parent, wxSizer* sizer = nullptr);
			void ClearView();
			void LoadView();
			void UpdateUI();

			DisplayModelModNode* ModToNode(const IGameMod& mod);

		private:
			bool CanShowPriorityGroups() const
			{
				return m_ShowPriorityGroups;
			}

			void OnSelectItem(KxDataView2::Event& event);
			void OnActivateItem(KxDataView2::Event& event);
			void OnExpandCollapseItem(KxDataView2::Event& event);
			void OnContextMenu(KxDataView2::Event& event);
			void OnHeaderContextMenu(KxDataView2::Event& event);
			
			bool CanStartDragOperation() const;
			IGameMod* TestDNDNode(KxDataView2::Node& node, bool allowPriorityGroup = false) const;
			void OnDragItems(KxDataView2::EventDND& event);
			void OnDropItems(KxDataView2::EventDND& event);
			void OnDropItemsPossible(KxDataView2::EventDND& event);

		public:
			DisplayModel();

		public:
			DisplayModelType GetDisplayMode() const
			{
				return m_DisplayMode;
			}
			void SetDisplayMode(DisplayModelType mode);

			bool ShouldShowPriorityGroups() const
			{
				return m_ShowPriorityGroups;
			}
			void ShowPriorityGroups(bool value)
			{
				m_ShowPriorityGroups = value;
			}
		
			bool ShouldShowNotInstalledMods() const
			{
				return m_ShowNotInstalledMods;
			}
			void ShowNotInstalledMods(bool value)
			{
				m_ShowNotInstalledMods = value;
			}

			bool IsBoldPriorityGroupLabels() const
			{
				return m_BoldPriorityGroupLabels;
			}
			void SetBoldPriorityGroupLabels(bool value)
			{
				m_BoldPriorityGroupLabels = value;
			}

			PriorityGroupLabelAlignment GetPriorityGroupLabelAlignment() const;
			void SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment value);

			void CreateSearchColumnsMenu(KxMenu& menu);
			bool SetSearchMask(const wxString& mask)
			{
				KAux::SetSearchMask(m_SearchMask, mask);
				return true;
			}
			void SetSearchColumns(const std::vector<KxDataView2::Column*>& columns);
			bool FilterMod(const IGameMod& mod) const;

			void SelectMod(const IGameMod* mod);
			IGameMod* GetSelectedMod() const;
	};
}
