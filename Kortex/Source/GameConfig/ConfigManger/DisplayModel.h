#pragma once
#include "stdafx.h"
#include "Item.h"
#include "Items/CategoryItem.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex
{
	class ITranslator;
	class IConfigManager;
}

namespace Kortex::GameConfig
{
	class DisplayModel: public KxRTTI::ExtendInterface<DisplayModel, KxDataView2::Model>
	{
		private:
			IConfigManager& m_Manager;
			const ITranslator& m_Translator;

			std::unordered_map<wxString, CategoryItem> m_Categories;
			bool m_ExpandBranches = false;
			bool m_DisableColumnsMenu = false;

		protected:
			virtual bool OnAskRefreshView();
			void ExpandAllCategories();

		private:
			void OnActivate(KxDataView2::Event& event);
			void OnContextMenu(KxDataView2::Event& event);

		public:
			DisplayModel(IConfigManager& manager);
			~DisplayModel();
		
		public:
			void CreateView(wxWindow* parent, wxSizer* sizer = nullptr);
			void ClearView();
			void LoadView();
			void RefreshView();

			void ExpandBranches(bool value = true)
			{
				m_ExpandBranches = value;
			}
			void DisableColumnsMenu(bool value = true)
			{
				m_DisableColumnsMenu = value;
			}
	};
}
