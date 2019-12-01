#pragma once
#include "stdafx.h"
#include "PageBase.h"
#include "Network/Common.h"
class KxTextBox;
class KxListBox;
class KxComboBox;
class KxSlider;

namespace Kortex
{
	class Workspace;
}
namespace Kortex::PackageProject
{
	class InfoSection;
	class ConfigSection;
}
namespace Kortex::PackageDesigner::PageInfoNS
{
	class TagsListModel;
}

namespace Kortex::PackageDesigner
{
	class PageInfo: public PageBase
	{
		friend class Workspace;

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

			PageInfoNS::TagsListModel* m_TagsModel = nullptr;

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

		private:
			PackageProject::InfoSection& GetProjectInfo() const;
			PackageProject::ConfigSection& GetProjectConfig() const;
			void OnLoadProject(PackageProject::InfoSection& tProjectInfo);

			void CreateBasicInfoControls();
			void CreateSitesControls();
			void CreateConfigControls();
			void CalculateMemoryRequiredForCompression(int nPower);
			template<class T> void OnOpenSite(wxTextUrlEvent& event)
			{
				ModSourceStore& store = GetProjectInfo().GetModSourceStore();

				KxURI uri = store.GetModPageURI(T::GetInstance()->GetName());
				if (uri.IsOk())
				{
					KAux::AskOpenURL(uri, this);
				}
				event.Skip();
			}
			template<class T> void OnEditSite(wxCommandEvent& event)
			{
				KxTextBox* textBox = static_cast<KxTextBox*>(event.GetEventObject());
				ModSourceStore& store = GetProjectInfo().GetModSourceStore();

				NetworkModInfo modInfo;
				modInfo.FromString(textBox->GetValue());
				store.AssignWith<T>(modInfo);

				event.Skip();
			}
			template<class T> KxTextBox* AddModSourceControl(wxSizer* sizer)
			{
				IModNetwork* modNetwork = T::GetInstance();
				if (modNetwork)
				{
					wxString name = modNetwork->GetName().BeforeLast('.');
					if (name.IsEmpty())
					{
						name = modNetwork->GetName();
					}

					KxLabel* label = nullptr;
					KxTextBox* control = AddControlsRow(sizer, name + "ID", CreateInputField(m_Pane), 1, &label);
					control->SetValidator(NetworkModInfo::GetValidator());

					label->ToggleWindowStyle(KxLABEL_HYPERLINK);
					label->SetBitmap(ImageProvider::GetBitmap(modNetwork->GetIcon()));
					label->Bind(wxEVT_TEXT_URL, &PageInfo::OnOpenSite<T>, this);

					return control;
				}
				return nullptr;
			};

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;

		public:
			PageInfo(Workspace& mainWorkspace, WorkspaceDocument& controller);
			~PageInfo();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::InformationFrame;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}
