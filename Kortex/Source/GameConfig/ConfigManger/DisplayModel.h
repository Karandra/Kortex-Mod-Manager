#pragma once
#include "stdafx.h"
#include "UI/KMainWindow.h"
#include "Item.h"
#include "Items/CategoryItem.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex
{
	class ITranslator;
	class IGameConfigManager;
}

namespace Kortex::GameConfig
{
	class DisplayModel: public RTTI::IExtendInterface<DisplayModel, KxDataView2::Model>
	{
		private:
			IGameConfigManager& m_Manager;
			const ITranslator& m_Translator;

			std::unordered_map<wxString, CategoryItem> m_Categories;

		protected:
			void OnDeleteNode(KxDataView2::Node* node) override;
			void OnDetachRootNode(KxDataView2::RootNode& node) override;

		private:
			void OnActivate(KxDataView2::Event& event);

		public:
			DisplayModel();
			~DisplayModel();
		
		public:
			void CreateView(wxWindow* parent, wxSizer* sizer);
			void ClearView();
			void LoadView();
			void RefreshView();
	};
}
