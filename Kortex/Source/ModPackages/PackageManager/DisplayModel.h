#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include "Utility/KDataViewListModel.h"
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxMenu.h>
class KxPanel;
class KxHTMLWindow;

namespace Kortex
{
	class ModPackage;
}

namespace Kortex::PackageManager
{
	class DisplayModelExtractionOperation;
	class DisplayModel: public KxDataViewListModelEx
	{
		friend class DisplayModelExtractionOperation;

		private:
			bool m_AutoShowPackageInfo = false;
			std::vector<KxFileItem> m_Data;
			std::unique_ptr<ModPackage> m_Package;

			KxMenu m_ContextMenu;
			KxMenuItem* m_ContextMenu_Open = nullptr;
			KxMenuItem* m_ContextMenu_ImportProject = nullptr;
			KxMenuItem* m_ContextMenu_ExtractFiles = nullptr;

			wxString m_PrevPath;
			wxString m_CurrentPath;
			bool m_IsRoot = true;
			KxImageView* m_ImageView = nullptr;
			KxHTMLWindow* m_Description = nullptr;

		private:
			void OnInitControl() override;

			void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
			bool HasDefaultCompare() const override
			{
				return true;
			}
			bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

			void CreateContextMenu();
			void RunInstallWizard(const KxFileItem& entry);
			wxBitmap GetIcon(const KxFileItem& entry) const;
			wxString GetType(const KxFileItem& entry) const;

		public:
			DisplayModel();

		public:
			size_t GetItemCount() const override;
			wxString GetHomePath() const
			{
				return IPackageManager::GetInstance()->GetPackagesFolder();
			}
		
			bool IsInRoot() const
			{
				return m_IsRoot;
			}
			void Navigate(const wxString& path);
			void NavigateUp();
			void NavigateHome();
			void Search(const wxString& mask);
		
			void RefreshCurrentView()
			{
				Navigate(m_CurrentPath);
			}
			void SetImageView(KxImageView* view)
			{
				m_ImageView = view;
			}
			void SetDescriptionView(KxHTMLWindow* view)
			{
				m_Description = view;
			}

			void LoadInfo();
			void ClearInfo();

			const KxFileItem* GetDataEntry(size_t index) const
			{
				if (index < m_Data.size())
				{
					return &m_Data.at(index);
				}
				return nullptr;
			}
			KxFileItem* GetDataEntry(size_t index)
			{
				if (index < m_Data.size())
				{
					return &m_Data.at(index);
				}
				return nullptr;
			}
			const KxFileItem* GetDataEntry(const KxDataViewItem& item) const
			{
				return GetDataEntry(GetRow(item));
			}
			KxFileItem* GetDataEntry(const KxDataViewItem& item)
			{
				return GetDataEntry(GetRow(item));
			}

			bool IsPackageOK() const;
			bool CanAutoShowPackageInfo() const
			{
				return m_AutoShowPackageInfo;
			}
	};
}

namespace Kortex::PackageManager
{
	class DisplayModelExtractionOperation: public KOperationWithProgressDialog<KxArchiveEvent>
	{
		private:
			DisplayModel* m_DisplayModel = nullptr;

		private:
			ModPackage& GetPackage() const
			{
				return *m_DisplayModel->m_Package;
			}

		public:
			DisplayModelExtractionOperation(const wxString& filePath, DisplayModel* model);
	};
}
