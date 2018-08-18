#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include "KDataViewListModel.h"
#include "KModPackage.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxMenu.h>
class KxPanel;
class KxHTMLWindow;
class KPackageManagerListModelThread;

class KPackageManagerListModel: public KDataViewListModel
{
	friend class KPackageManagerListModelThread;

	private:
		bool m_AutoShowPackageInfo = false;
		std::vector<KxFileFinderItem> m_Data;
		std::unique_ptr<KModPackage> m_Package;

		KxMenu m_ContextMenu;
		KxMenuItem* m_ContextMenu_Open = NULL;
		KxMenuItem* m_ContextMenu_ImportProject = NULL;
		KxMenuItem* m_ContextMenu_ExtractFiles = NULL;

		wxString m_PrevPath;
		wxString m_CurrentPath;
		bool m_IsRoot = true;
		KxImageView* m_ImageView = NULL;
		KxHTMLWindow* m_Description = NULL;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
		virtual bool HasDefaultCompare() const override
		{
			return false;
		}

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		void CreateContextMenu();
		void RunInstallWizard(const KxFileFinderItem& entry);
		wxBitmap GetIcon(const KxFileFinderItem& entry) const;
		wxString GetType(const KxFileFinderItem& entry) const;

	public:
		KPackageManagerListModel();

	public:
		virtual size_t GetItemCount() const override;
		wxString GetHomePath() const
		{
			return KPackageManager::Get().GetPackagesFolder();
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

		const KxFileFinderItem* GetDataEntry(size_t index) const
		{
			if (index < m_Data.size())
			{
				return &m_Data.at(index);
			}
			return NULL;
		}
		KxFileFinderItem* GetDataEntry(size_t index)
		{
			if (index < m_Data.size())
			{
				return &m_Data.at(index);
			}
			return NULL;
		}
		const KxFileFinderItem* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
		KxFileFinderItem* GetDataEntry(const KxDataViewItem& item)
		{
			return GetDataEntry(GetRow(item));
		}

		bool IsPackageOK() const
		{
			return m_Package != NULL && m_Package->IsOK();
		}
		bool CanAutoShowPackageInfo() const
		{
			return m_AutoShowPackageInfo;
		}
};

//////////////////////////////////////////////////////////////////////////
class KPackageManagerListModelThread: public KOperationWithProgressDialog<KxArchiveEvent>
{
	private:
		KPackageManagerListModel* m_Model = NULL;

	private:
		KModPackage& GetPackage() const
		{
			return *m_Model->m_Package;
		}

	public:
		KPackageManagerListModelThread(const wxString& filePath, KPackageManagerListModel* model);
};
