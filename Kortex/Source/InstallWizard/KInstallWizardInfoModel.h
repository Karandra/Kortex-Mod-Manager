#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include "KDataViewListModel.h"
#include "PackageProject/KPackageProject.h"
class KInstallWizardDialog;

namespace Kortex
{
	class ModTagStore;
}

enum KIWITypes
{
	KIWI_TYPE_NONE = 0,
	KIWI_TYPE_ID,
	KIWI_TYPE_NAME,
	KIWI_TYPE_SITE,
	KIWI_TYPE_TAGS,
};

class KInstallWizardInfoModel: public KDataViewListModel
{
	struct Item 
	{
		KLabeledValue Value;
		KIWITypes Type = KIWI_TYPE_NONE;
		KImageEnum IconID = KIMG_NONE;

		Item(const KLabeledValue& value)
			:Value(value)
		{
		}
	};
	typedef std::vector<Item> ItemVector;

	private:
		KInstallWizardDialog& m_InstallWizard;
		KPackageProject& m_Config;
		ItemVector m_DataVector;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

		bool CheckModID(const wxString& id);
		Kortex::ModTagStore& GetTagStore() const;
		void OnActivateItem(KxDataViewEvent& event);

	public:
		KInstallWizardInfoModel(KInstallWizardDialog& installWizard, KPackageProject& config)
			:m_InstallWizard(installWizard), m_Config(config)
		{
		}

	public:
		void AddItem(const KLabeledValue& value, KImageEnum image = KIMG_NONE, KIWITypes type = KIWI_TYPE_NONE);

		size_t GetItemCount() const override
		{
			return m_DataVector.size();
		}
		
		const Item* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return &m_DataVector.at(index);
			}
			return nullptr;
		}
		Item* GetDataEntry(size_t index)
		{
			if (index < GetItemCount())
			{
				return &m_DataVector.at(index);
			}
			return nullptr;
		}
		const Item* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
		Item* GetDataEntry(const KxDataViewItem& item)
		{
			return GetDataEntry(GetRow(item));
		}
};
