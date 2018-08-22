#pragma once
#include "stdafx.h"
#include "KPluginViewModel.h"
class KPluginEntry;

class KPluginViewModelBethesda: public KPluginViewModel
{
	protected:
		virtual void OnInitControl() override;

	public:
		virtual void GetValue(wxAny& value, const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const override;
		virtual bool SetValue(const wxAny& value, KPluginEntry& pluginEntry, const KxDataViewColumn* column) override;

		virtual bool IsEnabled(const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const override;
		virtual bool IsEditorEnabled(const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const override;

		virtual bool GetAttributes(const KPluginEntry& pluginEntry, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
		virtual bool Compare(const KPluginEntry& pluginEntry1, const KPluginEntry& pluginEntry2, const KxDataViewColumn* column) const override;

	public:
		const wxString& GetPartOfName(const KPluginEntry& entry) const;
		wxString GetPluginAuthor(const KPluginEntry& entry) const;
};
