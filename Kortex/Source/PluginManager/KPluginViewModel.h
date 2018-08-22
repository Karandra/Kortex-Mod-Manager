#pragma once
#include "stdafx.h"
#include <KxFramework/KxDataView.h>
class KPluginEntry;

class KPluginViewModel
{
	friend class KPluginViewBaseModel;

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
		KxDataViewCtrl* m_View = NULL;

	private:
		void SetView(KxDataViewCtrl* view)
		{
			m_View = view;
		}

	protected:
		virtual void OnInitControl() = 0;

	public:
		virtual void GetValue(wxAny& value, const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const = 0;
		virtual bool SetValue(const wxAny& value, KPluginEntry& pluginEntry, const KxDataViewColumn* column) = 0;

		virtual bool IsEnabled(const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const = 0;
		virtual bool IsEditorEnabled(const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const = 0;

		virtual bool GetAttributes(const KPluginEntry& pluginEntry, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const = 0;
		virtual bool Compare(const KPluginEntry& pluginEntry1, const KPluginEntry& pluginEntry2, const KxDataViewColumn* column) const = 0;

	public:
		KxDataViewCtrl* GetView() const
		{
			return m_View;
		}
};
