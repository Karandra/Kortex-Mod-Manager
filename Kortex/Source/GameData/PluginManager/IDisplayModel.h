#pragma once
#include "stdafx.h"
#include <KxFramework/KxDataView.h>

namespace Kortex
{
	class IPluginManager;
	class IGamePlugin;
}

namespace Kortex::PluginManager
{
	class IDisplayModel
	{
		friend class PluginViewModel;

		public:
			enum ColumnID
			{
				Name,
				Index,
				Type,
				PartOf,
				Author,

				MIN_USER,
			};

		private:
			KxDataViewCtrl* m_View = nullptr;

		private:
			void SetView(KxDataViewCtrl* view)
			{
				m_View = view;
			}

		protected:
			virtual void OnInitControl() = 0;

		public:
			virtual void GetValue(wxAny& value, const IGamePlugin& plugin, const KxDataViewColumn* column) const = 0;
			virtual bool SetValue(const wxAny& value, IGamePlugin& plugin, const KxDataViewColumn* column) = 0;

			virtual bool IsEnabled(const IGamePlugin& plugin, const KxDataViewColumn* column) const = 0;
			virtual bool IsEditorEnabled(const IGamePlugin& plugin, const KxDataViewColumn* column) const = 0;

			virtual bool GetAttributes(const IGamePlugin& plugin, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const = 0;
			virtual bool Compare(const IGamePlugin& plugin1, const IGamePlugin& plugin2, const KxDataViewColumn* column) const = 0;

		public:
			KxDataViewCtrl* GetView() const
			{
				return m_View;
			}
	};
}
