#include "stdafx.h"
#include "KPackageCreatorPageInfo.h"
#include "KPackageCreatorPageBase.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorController.h"
#include "PageInfo/KPCInfoDocumentsModel.h"
#include "PageInfo/KPCInfoTagsListModel.h"
#include "PageInfo/KPCInfoSitesModel.h"
#include "PageInfo/KPCInfoAdditionalInfoModel.h"
#include "PackageProject/KPackageProject.h"
#include "PackageProject/KPackageProjectInfo.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModWorkspace.h"
#include "ModManager/KModEntry.h"
#include "Network/KNetwork.h"
#include "Network/KNetworkProviderNexus.h"
#include "Network/KNetworkProviderTESALL.h"
#include "Network/KNetworkProviderLoversLab.h"
#include "UI/KTextEditorDialog.h"
#include "KThemeManager.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxSlider.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxCollapsiblePane.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxStdDialogSimple.h>
#pragma warning(disable: 4302)
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

KPackageCreatorPageInfo::KPackageCreatorPageInfo(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller)
	:KPackageCreatorPageBase(mainWorkspace, controller)
{
}
KPackageCreatorPageInfo::~KPackageCreatorPageInfo()
{
}
bool KPackageCreatorPageInfo::OnCreateWorkspace()
{
	m_PaneSizer = new wxBoxSizer(wxVERTICAL);
	m_Pane = new KxScrolledPanel(this, KxID_NONE, wxDefaultPosition, wxDefaultSize);
	m_Pane->SetSizer(m_PaneSizer);

	m_MainSizer->Add(m_Pane, 1, wxEXPAND);
	KThemeManager::Get().ProcessWindow(m_Pane);

	CreateBasicInfoControls();
	CreateSitesControls();
	CreateConfigControls();

	m_Pane->FitInside();
	m_Pane->SetScrollRate(0, 10);
	return true;
}
KPackageProjectInfo& KPackageCreatorPageInfo::GetProjectInfo() const
{
	return GetProject()->GetInfo();
}
KPackageProjectConfig& KPackageCreatorPageInfo::GetProjectConfig() const
{
	return GetProject()->GetConfig();
}

void KPackageCreatorPageInfo::CreateBasicInfoControls()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(m_Pane, KTr("PackageCreator.PageInfo.BasicInfo"));
	m_PaneSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	wxFlexGridSizer* basicInfoSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, 0);
	basicInfoSizer->AddGrowableCol(1, 1);
	m_PaneSizer->Add(basicInfoSizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

	// Main info
	wxBoxSizer* nameIDSizer = new wxBoxSizer(wxHORIZONTAL);
	AddControlsRow2(m_Pane, basicInfoSizer, KTr("PackageCreator.PageInfo.BasicInfo.Name"), nameIDSizer, 2);

	m_NameInput = CreateInputField(m_Pane);
	nameIDSizer->Add(m_NameInput, 1, wxEXPAND|wxRIGHT, 6 * KLC_HORIZONTAL_SPACING);

	m_IDInput = AddControlsRow(nameIDSizer, KTr("PackageCreator.PageInfo.BasicInfo.ID"), CreateInputField(m_Pane));
	m_IDInput->SetMaxSize(wxSize(150, -1));

	m_VersionInput = AddControlsRow(basicInfoSizer, KTr("PackageCreator.PageInfo.BasicInfo.Version"), CreateInputField(m_Pane));
	m_AuthorInput = AddControlsRow(basicInfoSizer, KTr("PackageCreator.PageInfo.BasicInfo.Author"), CreateInputField(m_Pane));

	// Tags
	{
		wxBoxSizer* sizer = AddControlsRow2(m_Pane, basicInfoSizer, KTr("PackageCreator.PageInfo.BasicInfo.Tags"), new wxBoxSizer(wxVERTICAL));
		m_TagsModel = new KPCInfoTagsListModel();
		m_TagsModel->Create(m_Controller, m_Pane, sizer);
	}

	m_DescriptionButton = AddControlsRow(basicInfoSizer, KTr("PackageCreator.PageInfo.BasicInfo.Description"), new KxButton(m_Pane, KxID_NONE, KTr(KxID_EDIT)), 0);

	// Additional info
	wxBoxSizer* collapsePaneContainerSizer = new wxBoxSizer(wxVERTICAL);
	m_PaneSizer->Add(collapsePaneContainerSizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);
	KxCollapsiblePane* pCollapsePane = new KxCollapsiblePane(m_Pane, KxID_NONE, KTr("PackageCreator.PageInfo.BasicInfo.AdditionalData"));
	collapsePaneContainerSizer->Add(pCollapsePane, 0, wxEXPAND);

	wxFlexGridSizer* collapsePaneSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, 0);
	collapsePaneSizer->AddGrowableCol(1, 1);
	collapsePaneSizer->SetSizeHints(pCollapsePane->GetPane());
	pCollapsePane->GetPane()->SetSizer(collapsePaneSizer);

	m_TranslatedNameInput = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.BasicInfo.TranslatedName"), CreateInputField(pCollapsePane->GetPane()));
	m_TranslatorNameInput = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.BasicInfo.Translatior"), CreateInputField(pCollapsePane->GetPane()));
	m_UserDataButton = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.BasicInfo.UserData"), new KxButton(pCollapsePane->GetPane(), KxID_NONE, KTr(KxID_EDIT)), 0);
	m_DocumentsButton = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.BasicInfo.Documents"), new KxButton(pCollapsePane->GetPane(), KxID_NONE, KTr(KxID_EDIT)), 0);

	// Bind events
	m_IDInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProject()->SetModID(m_IDInput->GetValue());
		event.Skip();
	});
	m_NameInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProjectInfo().SetName(m_NameInput->GetValue());
		event.Skip();
	});
	m_TranslatedNameInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProjectInfo().SetTranslatedName(m_TranslatedNameInput->GetValue());
		event.Skip();
	});
	m_VersionInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProjectInfo().SetVersion(m_VersionInput->GetValue());
		event.Skip();
	});
	m_AuthorInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProjectInfo().SetAuthor(m_AuthorInput->GetValue());
		event.Skip();
	});
	m_TranslatorNameInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProjectInfo().SetTranslator(m_TranslatorNameInput->GetValue());
		event.Skip();
	});
	
	m_DescriptionButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		KTextEditorDialog dialog(GetMainWindow());
		dialog.SetText(GetProjectInfo().GetDescription());
		if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
		{
			GetProjectInfo().SetDescription(dialog.GetText());
			m_Controller->ChangeNotify();
		}
		event.Skip();
	});
	m_UserDataButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		KPCInfoAdditionalInfoModelDialog dialog(GetMainWindow(), KTr("PackageCreator.PageInfo.BasicInfo.UserData"), m_Controller, true);
		dialog.SetDataVector(GetProjectInfo().GetCustomFields(), &GetProjectInfo());
		dialog.ShowModal();
		event.Skip();
	});
	m_DocumentsButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		KPCInfoDocumentsModelDialog dialog(GetMainWindow(), KTr("PackageCreator.PageInfo.BasicInfo.Documents"), m_Controller);
		dialog.SetDataVector(GetProjectInfo().GetDocuments(), &GetProjectInfo());
		dialog.ShowModal();
		event.Skip();
	});
}
void KPackageCreatorPageInfo::CreateSitesControls()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(m_Pane, KTr("PackageCreator.PageInfo.Sites"));
	m_PaneSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	wxFlexGridSizer* sitesSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, 0);
	sitesSizer->AddGrowableCol(1, 1);
	m_PaneSizer->Add(sitesSizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

	// Fixed sites
	m_WebSitesNexusID = AddProviderControl<KNetworkProviderNexus>(sitesSizer);
	m_WebSitesTESALLID = AddProviderControl<KNetworkProviderTESALL>(sitesSizer);
	m_WebSitesLoversLabID = AddProviderControl<KNetworkProviderLoversLab>(sitesSizer);

	// Other sites
	m_WebSitesButton = AddControlsRow(sitesSizer, KTr("PackageCreator.PageInfo.Sites.AdditionalSites"), new KxButton(m_Pane, KxID_NONE, KTr(KxID_EDIT)), 0);

	// Bind events
	m_WebSitesTESALLID->Bind(wxEVT_TEXT, &KPackageCreatorPageInfo::OnEditSite<KNETWORK_PROVIDER_ID_TESALL>, this);
	m_WebSitesNexusID->Bind(wxEVT_TEXT, &KPackageCreatorPageInfo::OnEditSite<KNETWORK_PROVIDER_ID_NEXUS>, this);
	m_WebSitesLoversLabID->Bind(wxEVT_TEXT, &KPackageCreatorPageInfo::OnEditSite<KNETWORK_PROVIDER_ID_LOVERSLAB>, this);

	m_WebSitesButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		KPCInfoSitesModelDialog dialog(GetMainWindow(), KTr("PackageCreator.PageInfo.Sites.AdditionalSites"), m_Controller, true);
		dialog.SetDataVector(GetProjectInfo().GetWebSites(), &GetProjectInfo());
		dialog.ShowModal();
		event.Skip();
	});
}
void KPackageCreatorPageInfo::CreateConfigControls()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(m_Pane, KTr("PackageCreator.PageInfo.Config"));
	m_PaneSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	wxFlexGridSizer* configSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, 0);
	configSizer->AddGrowableCol(1, 1);
	m_PaneSizer->Add(configSizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

	/* Required parameters */
	// Package path
	wxBoxSizer* pathSizer = AddControlsRow2(m_Pane, configSizer, KTr("PackageCreator.PageInfo.Config.PackageOutputFile"), new wxBoxSizer(wxHORIZONTAL));
	m_InstallBackageFileInput = CreateInputField(m_Pane);
	KxButton* installBackageFileButton = new KxButton(m_Pane, KxID_NONE, KTr(KxID_SELECT_FILE));

	pathSizer->Add(m_InstallBackageFileInput, 1, wxEXPAND);
	pathSizer->Add(installBackageFileButton, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

	// Compression level
	m_CompressionLevel = AddControlsRow(configSizer, KTr("PackageCreator.PageInfo.Config.CompressionLevel"), new KxComboBox(m_Pane, KxID_NONE));
	auto AddCompressionLevel = [this](const wxString& id, int level)
	{
		m_CompressionLevel->Append(wxString::Format("%d - %s", level, KTr(id)), reinterpret_cast<void*>(level));
	};
	AddCompressionLevel("PackageCreator.PageInfo.Config.CompressionLevel.None", 0);
	AddCompressionLevel("PackageCreator.PageInfo.Config.CompressionLevel.Fastest", 1);
	AddCompressionLevel("PackageCreator.PageInfo.Config.CompressionLevel.Fast", 3);
	AddCompressionLevel("PackageCreator.PageInfo.Config.CompressionLevel.Normal", 5);
	AddCompressionLevel("PackageCreator.PageInfo.Config.CompressionLevel.Maximum", 7);
	AddCompressionLevel("PackageCreator.PageInfo.Config.CompressionLevel.Ultra", 9);

	/* Advanced */
	wxBoxSizer* collapsePaneContainerSizer = new wxBoxSizer(wxVERTICAL);
	m_PaneSizer->Add(collapsePaneContainerSizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);
	KxCollapsiblePane* collapsePane = new KxCollapsiblePane(m_Pane, KxID_NONE, KTr("PackageCreator.PageInfo.Config.CompressionAdvanced"));
	collapsePaneContainerSizer->Add(collapsePane, 0, wxEXPAND);

	wxFlexGridSizer* collapsePaneSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, 0);
	collapsePaneSizer->AddGrowableCol(1, 1);
	collapsePaneSizer->SetSizeHints(collapsePane->GetPane());
	collapsePane->GetPane()->SetSizer(collapsePaneSizer);

	// Compression Method
	m_CompressionMethod = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.Config.CompressionMethod"), new KxComboBox(collapsePane->GetPane(), KxID_NONE));
	m_CompressionMethod->AddItem("LZMA");
	m_CompressionMethod->AddItem("LZMA2");
	m_CompressionMethod->AddItem("PPMd");
	m_CompressionMethod->AddItem("BZip2");

	// Dictionary size
	m_CompressionDictionarySize = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.Config.DictionarySize"), new KxSlider(collapsePane->GetPane(), KxID_NONE, KPackageProjectConfig::ms_DefaultDictionarySize, KPackageProjectConfig::ms_MinDictionarySize, KPackageProjectConfig::ms_MaxDictionarySize, KxSlider::DefaultStyle|wxSL_TICKS));
	m_CompressionDictionarySize->SetMaxSize(wxSize(wxDefaultCoord, 23));
	m_CompressionDictionarySizeMemory = AddControlsRow(collapsePaneSizer, KTr("PackageCreator.PageInfo.Config.DictionarySize.Memory"), CreateNormalLabel(collapsePane->GetPane(), wxEmptyString));

	// Checkboxes
	m_CompressionUseMultithreading = new wxCheckBox(collapsePane->GetPane(), KxID_ANY, KTr("PackageCreator.PageInfo.Config.UseMultithreadedCompression"));
	collapsePaneSizer->Add(m_CompressionUseMultithreading, 1, wxEXPAND);

	m_CompressionSolidArchive = new wxCheckBox(collapsePane->GetPane(), KxID_ANY, KTr("PackageCreator.PageInfo.Config.SolidArchive"));
	collapsePaneSizer->Add(m_CompressionSolidArchive, 1, wxEXPAND);

	// Bind events
	m_InstallBackageFileInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& event)
	{
		GetProjectConfig().SetInstallPackageFile(m_InstallBackageFileInput->GetValue());
		event.Skip();
	});
	installBackageFileButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		wxString name;
		wxString folder = GetProjectConfig().GetInstallPackageFile().BeforeLast('\\', &name);
		name = name.BeforeLast('.');
		if (name.IsEmpty())
		{
			name = m_Controller->GetProjectName();
		}

		KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_SAVE);
		dialog.SetOptionEnabled(KxFBD_DONT_ADD_TO_RECENT);
		dialog.SetOptionEnabled(KxFBD_NO_CHANGE_CWD);
		dialog.SetDefaultExtension("kmp");
		dialog.SetFolder(folder);
		dialog.SetFileName(name);
		dialog.AddFilter("*.kmp", KTr("FileFilter.ModPackage"));
		dialog.AddFilter("*.7z", KTr("FileFilter.7ZipArchive"));
		dialog.AddFilter("*.fomod", KTr("FileFilter.ModPackageFOMod"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

		if (dialog.ShowModal() == KxID_OK)
		{
			wxString path = dialog.GetResult();
			GetProjectConfig().SetInstallPackageFile(path);
			m_InstallBackageFileInput->SetValue(path);

			m_Controller->ChangeNotify();
		}
		event.Skip();
	});
	m_CompressionMethod->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
	{
		GetProjectConfig().SetCompressionMethod(m_CompressionMethod->GetItemLabel(event.GetInt()));
		CalculateMemoryRequiredForCompression(GetProjectConfig().GetCompressionDictionarySize());
		m_Controller->ChangeNotify();
		event.Skip();
	});
	m_CompressionLevel->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
	{
		int level = reinterpret_cast<int>(m_CompressionLevel->GetClientData(event.GetInt()));
		GetProjectConfig().SetCompressionLevel(level);
		m_Controller->ChangeNotify();
		event.Skip();
	});
	m_CompressionDictionarySize->Bind(wxEVT_SLIDER, [this](wxCommandEvent& event)
	{
		int nPower = event.GetInt();
		CalculateMemoryRequiredForCompression(nPower);
		GetProjectConfig().SetCompressionDictionarySize(nPower);
		m_Controller->ChangeNotify();
		event.Skip();
	});
	m_CompressionUseMultithreading->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event)
	{
		GetProjectConfig().SetUseMultithreading(event.IsChecked());
		m_Controller->ChangeNotify();
		event.Skip();
	});
	m_CompressionSolidArchive->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event)
	{
		GetProjectConfig().SetSolidArchive(event.IsChecked());
		m_Controller->ChangeNotify();
		event.Skip();
	});
}

bool KPackageCreatorPageInfo::OnOpenWorkspace()
{
	return true;
}
bool KPackageCreatorPageInfo::OnCloseWorkspace()
{
	return true;
}
void KPackageCreatorPageInfo::OnLoadProject(KPackageProjectInfo& tProjectInfo)
{
	wxWindowUpdateLocker lock(this);

	/* Basic info */
	m_IDInput->SetValue(tProjectInfo.GetProject().GetModID());
	m_NameInput->SetValue(tProjectInfo.GetName());
	m_TranslatedNameInput->SetValue(tProjectInfo.GetTranslatedName());
	m_VersionInput->SetValue(tProjectInfo.GetVersion());
	m_AuthorInput->SetValue(tProjectInfo.GetAuthor());
	m_TranslatorNameInput->SetValue(tProjectInfo.GetTranslator());
	m_TagsModel->SetDataVector(&tProjectInfo.GetTags());

	/* Web sites */
	auto LoadFixedSite = [tProjectInfo](KNetworkProviderID index, KxTextBox* input)
	{
		if (tProjectInfo.HasWebSite(index))
		{
			input->SetValue(std::to_wstring(tProjectInfo.GetWebSiteModID(index)));
		}
		else
		{
			input->Clear();
		}
	};
	LoadFixedSite(KNETWORK_PROVIDER_ID_TESALL, m_WebSitesTESALLID);
	LoadFixedSite(KNETWORK_PROVIDER_ID_NEXUS, m_WebSitesNexusID);
	LoadFixedSite(KNETWORK_PROVIDER_ID_LOVERSLAB, m_WebSitesLoversLabID);

	/* Config */
	// Package path
	m_InstallBackageFileInput->SetValue(GetProjectConfig().GetInstallPackageFile());

	// Compression method
	m_CompressionMethod->SetSelection(0);
	m_CompressionMethod->SetValue(GetProjectConfig().GetCompressionMethod());

	// Compression level
	int compressionLevel = GetProjectConfig().GetCompressionLevel();
	for (unsigned int i = 0; i < m_CompressionLevel->GetCount(); i++)
	{
		if (reinterpret_cast<int>(m_CompressionLevel->GetClientData(i)) == compressionLevel)
		{
			m_CompressionLevel->SetSelection(i);
			break;
		}
	}

	// Dictionary size
	int dictionarySizePower = GetProjectConfig().GetCompressionDictionarySize();
	m_CompressionDictionarySize->SetValue(dictionarySizePower);
	CalculateMemoryRequiredForCompression(dictionarySizePower);

	// Checkboxes
	m_CompressionUseMultithreading->SetValue(GetProjectConfig().IsMultithreadingUsed());
	m_CompressionSolidArchive->SetValue(GetProjectConfig().IsSolidArchive());
}
void KPackageCreatorPageInfo::CalculateMemoryRequiredForCompression(int nPower)
{
	// Roughly calculate required memory
	const int coresCount = 1;
	const int memoryPerThread = 32;
	int64_t memoryMB = std::pow(2, nPower);

	wxString out;
	int64_t requiredMemoryMB = 0;
	bool isLessThan = false;
	const wxString& method = GetProjectConfig().GetCompressionMethod();
	if (method == "LZMA" || method == "LZMA2")
	{
		if (nPower < 3)
		{
			requiredMemoryMB = memoryPerThread + memoryMB * 11;
		}
		else
		{
			requiredMemoryMB = memoryMB * 11;
		}
	}
	else if (method == "PPMd")
	{
		requiredMemoryMB = memoryMB + memoryPerThread - 1;
	}
	else if (method == "BZip2")
	{
		requiredMemoryMB = 128;
		isLessThan = true;
	}

	if (requiredMemoryMB != 0)
	{
		// nRequiredMemoryMB is in megabytes
		out = KxFile::FormatFileSize(memoryMB * 1024 * 1024);
		out << ' ' << wxUniChar(0x2192) << (isLessThan ? " < " : " ~") + KxFile::FormatFileSize(requiredMemoryMB * 1024 * 1024);
	}
	else
	{
		out = "N/A";
	}
	m_CompressionDictionarySizeMemory->SetLabel(out);
}

wxString KPackageCreatorPageInfo::GetID() const
{
	return "KPackageCreator.Info";
}
wxString KPackageCreatorPageInfo::GetPageName() const
{
	return KTr("PackageCreator.PageInfo.Name");
}
