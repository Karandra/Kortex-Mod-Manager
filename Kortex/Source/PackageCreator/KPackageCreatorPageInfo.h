#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorPageBase.h"
#include "KProgramOptions.h"
#include "Network/KNetworkConstants.h"
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
		KxScrolledPanel* m_Pane = NULL;
		wxBoxSizer* m_PaneSizer = NULL;

		// Basic info
		KxTextBox* m_NameInput = NULL;
		KxTextBox* m_IDInput = NULL;
		KxTextBox* m_TranslatedNameInput = NULL;
		KxTextBox* m_VersionInput = NULL;
		KxTextBox* m_AuthorInput = NULL;
		KxTextBox* m_TranslatorNameInput = NULL;
		KxButton* m_DescriptionButton = NULL;
		KxButton* m_UserDataButton = NULL;
		KxButton* m_DocumentsButton = NULL;

		KPCInfoTagsListModel* m_TagsModel = NULL;

		// WebSites
		KxTextBox* m_WebSitesTESALLID = NULL;
		KxTextBox* m_WebSitesNexusID = NULL;
		KxTextBox* m_WebSitesLoversLabID = NULL;
		KxButton* m_WebSitesButton = NULL;

		// Config
		KxTextBox* m_InstallBackageFileInput = NULL;
		KxComboBox* m_CompressionMethod = NULL;
		KxComboBox* m_CompressionLevel = NULL;
		KxSlider* m_CompressionDictionarySize = NULL;
		KxLabel* m_CompressionDictionarySizeMemory = NULL;
		wxCheckBox* m_CompressionUseMultithreading = NULL;
		wxCheckBox* m_CompressionSolidArchive = NULL;

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
		template<KNetworkProviderID index> void OnOpenSite(wxTextUrlEvent& event)
		{
			if (GetProjectInfo().HasWebSite(index))
			{
				KAux::AskOpenURL(GetProjectInfo().GetWebSite(index).GetValue(), this);
			}
			event.Skip();
		}
		template<KNetworkProviderID index> void OnEditSite(wxCommandEvent& event)
		{
			long long id = -1;
			static_cast<KxTextBox*>(event.GetEventObject())->GetValue().ToLongLong(&id);
			GetProjectInfo().SetWebSite(index, id);

			event.Skip();
		}
		template<class T> KxTextBox* AddProviderControl(wxSizer* sizer)
		{
			KNetworkProvider* provider = T::GetInstance();

			KxLabel* label = NULL;
			wxString name = provider->GetName().BeforeLast('.');
			if (name.IsEmpty())
			{
				name = provider->GetName();
			}

			KxTextBox* control = AddControlsRow(sizer, name + "ID", CreateInputField(m_Pane), 1, &label);
			label->ToggleWindowStyle(KxLABEL_HYPERLINK);
			label->SetBitmap(KGetBitmap(provider->GetIcon()));
			label->Bind(wxEVT_TEXT_URL, &KPackageCreatorPageInfo::OnOpenSite<T::GetTypeID()>, this);
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
