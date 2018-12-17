#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorPageBase.h"
#include "Network/NetworkConstants.h"
class KPackageCreatorWorkspace;
class KPackageProjectInfo;
class KPackageProjectConfig;
class KPCInfoTagsListModel;
class KxTextBox;
class KxListBox;
class KxComboBox;
class KxSlider;

class KPackageCreatorPageInfo: public KPackageCreatorPageBase
{
	friend class KPackageCreatorWorkspace;

	private:
		KxScrolledPanel* m_Pane = nullptr;
		wxBoxSizer* m_PaneSizer = nullptr;

		// Basic info
		KxTextBox* m_NameInput = nullptr;
		KxTextBox* m_IDInput = nullptr;
		KxTextBox* m_TranslatedNameInput = nullptr;
		KxTextBox* m_VersionInput = nullptr;
		KxTextBox* m_AuthorInput = nullptr;
		KxTextBox* m_TranslatorNameInput = nullptr;
		KxButton* m_DescriptionButton = nullptr;
		KxButton* m_UserDataButton = nullptr;
		KxButton* m_DocumentsButton = nullptr;

		KPCInfoTagsListModel* m_TagsModel = nullptr;

		// WebSites
		KxTextBox* m_WebSitesTESALLID = nullptr;
		KxTextBox* m_WebSitesNexusID = nullptr;
		KxTextBox* m_WebSitesLoversLabID = nullptr;
		KxButton* m_WebSitesButton = nullptr;

		// Config
		KxTextBox* m_InstallBackageFileInput = nullptr;
		KxComboBox* m_CompressionMethod = nullptr;
		KxComboBox* m_CompressionLevel = nullptr;
		KxSlider* m_CompressionDictionarySize = nullptr;
		KxLabel* m_CompressionDictionarySizeMemory = nullptr;
		wxCheckBox* m_CompressionUseMultithreading = nullptr;
		wxCheckBox* m_CompressionSolidArchive = nullptr;

	public:
		KPackageCreatorPageInfo(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller);
		virtual ~KPackageCreatorPageInfo();

	private:
		virtual bool OnCreateWorkspace() override;
		KPackageProjectInfo& GetProjectInfo() const;
		KPackageProjectConfig& GetProjectConfig() const;

		void CreateBasicInfoControls();
		void CreateSitesControls();
		void CreateConfigControls();
		void CalculateMemoryRequiredForCompression(int nPower);
		template<class T> void OnOpenSite(wxTextUrlEvent& event)
		{
			Kortex::ModProvider::Store& store = GetProjectInfo().GetProviderStore();

			wxString url = store.GetModURL(T::GetInstance()->GetName());
			if (!url.IsEmpty())
			{
				KAux::AskOpenURL(url, this);
			}
			event.Skip();
		}
		template<class T> void OnEditSite(wxCommandEvent& event)
		{
			long long id = -1;
			static_cast<KxTextBox*>(event.GetEventObject())->GetValue().ToLongLong(&id);

			Kortex::ModProvider::Store& store = GetProjectInfo().GetProviderStore();
			store.AssignWith<T>(id);
			event.Skip();
		}
		template<class T> KxTextBox* AddProviderControl(wxSizer* sizer)
		{
			Kortex::INetworkProvider* provider = T::GetInstance();

			KxLabel* label = nullptr;
			wxString name = provider->GetName().BeforeLast('.');
			if (name.IsEmpty())
			{
				name = provider->GetName();
			}

			KxTextBox* control = AddControlsRow(sizer, name + "ID", CreateInputField(m_Pane), 1, &label);
			label->ToggleWindowStyle(KxLABEL_HYPERLINK);
			label->SetBitmap(KGetBitmap(provider->GetIcon()));
			label->Bind(wxEVT_TEXT_URL, &KPackageCreatorPageInfo::OnOpenSite<T>, this);
			control->SetValidator(wxIntegerValidator<int64_t>());

			return control;
		};

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnLoadProject(KPackageProjectInfo& tProjectInfo);

	public:
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_INFORMATION_FRAME;
		}
		virtual wxString GetID() const override;
		virtual wxString GetPageName() const override;
};
