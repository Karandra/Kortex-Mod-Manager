#pragma once
#include "stdafx.h"
#include "Application/Resources/ImageResourceID.h"
#include "Utility/KDataViewListModel.h"
#include "PackageProject/KPackageProject.h"

namespace Kortex
{
	class ModTagStore;
}
namespace Kortex::InstallWizard
{
	class WizardDialog;
}

namespace Kortex::InstallWizard
{
	enum class InfoKind
	{
		None = 0,
		ID,
		Name,
		ModSource,
		Tags,
	};
}

namespace Kortex::InstallWizard
{
	class InfoDisplayModel: public KxDataViewListModelEx
	{
		struct Item 
		{
			KLabeledValue Value;
			InfoKind Type = InfoKind::None;
			ResourceID IconID;

			Item(const KLabeledValue& value)
				:Value(value)
			{
			}
		};
		typedef std::vector<Item> ItemVector;

		private:
			WizardDialog& m_InstallWizard;
			KPackageProject& m_Config;
			ItemVector m_DataVector;

		private:
			void OnInitControl() override;

			void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
			bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

			bool CheckModID(const wxString& id);
			ModTagStore& GetTagStore() const;
			void OnActivateItem(KxDataViewEvent& event);

		public:
			InfoDisplayModel(WizardDialog& installWizard, KPackageProject& config)
				:m_InstallWizard(installWizard), m_Config(config)
			{
			}

		public:
			void AddItem(const KLabeledValue& value, const ResourceID& image = {}, InfoKind type = InfoKind::None);

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
}
