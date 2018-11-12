#pragma once
#include "stdafx.h"
#include "PackageManager/KModPackage.h"
#include "ModManager/KModEntry.h"
#include "KInstallWizardStepStack.h"
#include "KOperationWithProgress.h"
#include "KTempFolderKeeper.h"
#include "KImageProvider.h"
#include "KProgramOptions.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxThumbView.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxProgressBar.h>
#include <KxFramework/KxWithOptions.h>
#include <KxFramework/KxDataView.h>
class KInstallWizardInfoModel;
class KInstallWizardRequirementsModel;
class KInstallWizardComponnetsModel;
class KImageViewerEvent;
class KIWDInstallOperation;
class KInstallWizardInfoModel;

wxDECLARE_EVENT(KEVT_IW_DONE, wxNotifyEvent);

enum KInstallWizardDialogOptions
{
	KIWD_OPTION_NONE = 0,
	KIWD_OPTION_CLEANUP = 1 << 0,
	KIWD_OPTION_PREVIEW = 1 << 1,
};
enum KInstallWizardPages
{
	KIWD_PAGE_NONE = -1,
	KIWD_PAGE_INFO,
	KIWD_PAGE_REQUIREMENTS,
	KIWD_PAGE_COMPONENTS,
	KIWD_PAGE_INSTALLING,
	KIWD_PAGE_DONE,
};

class KInstallWizardDialog:
	public KxStdDialog,
	public KTempFolderKeeper,
	public KxWithOptions<KInstallWizardDialogOptions, KIWD_OPTION_NONE>
{
	friend class KIWDInstallOperation;
	friend class KInstallWizardInfoModel;

	public:
		static void ShowInvalidPackageDialog(wxWindow* window, const wxString& packagePath);

	private:
		/* UI */
		KxButton* m_CancelButton = NULL;
		KxButton* m_BackwardButton = NULL;
		KxButton* m_ForwardButton = NULL;
		wxSimplebook* m_TabView = NULL;
		KInstallWizardPages m_CurrentPage = KIWD_PAGE_NONE;

		// Info
		KxAuiNotebook* m_Info_Tabs = NULL;
		KInstallWizardInfoModel* m_Info_PackageInfoList = NULL;
		KxHTMLWindow* m_Info_Description = NULL;

		KxSplitterWindow* m_Info_DocumentsSplitter = NULL;
		KxListBox* m_Info_DocumentsList = NULL;
		KxHTMLWindow* m_Info_DocumentSimple = NULL;
		wxWebView* m_Info_DocumentAdvanced = NULL;

		KxThumbView* m_Info_Screenshots = NULL;

		// Main requirements
		KInstallWizardRequirementsModel* m_Requirements_Main = NULL;

		// Components
		KxSplitterWindow* m_Components_SplitterV = NULL;
		KxSplitterWindow* m_Components_SplitterHRight = NULL;

		KInstallWizardComponnetsModel* m_Components_ItemList = NULL;
		KxAuiNotebook* m_Components_Tabs = NULL;
		KxImageView* m_Components_ImageView = NULL;
		KxHTMLWindow* m_Components_Description = NULL;
		KInstallWizardRequirementsModel* m_Components_Requirements = NULL;

		// Installing
		KxPanel* m_Installing_Pane = NULL;
		KxProgressBar* m_Installing_MinorProgress = NULL;
		KxProgressBar* m_Installing_MajorProgress = NULL;
		KxLabel* m_Installing_MinorStatus = NULL;
		KxLabel* m_Installing_MajorStatus = NULL;
		bool m_ShouldCancel = false;
		bool m_IsComplete = false;
		bool m_InfoPageLeft = false;
		KIWDInstallOperation* m_InstallThread = NULL;

		// Done
		KxPanel* m_Done_Pane = NULL;
		KxLabel* m_Done_Label = NULL;

		/* UI options */
		KProgramOptionUI m_Option_Window;
		KProgramOptionUI m_Option_MainUI;
		KProgramOptionUI m_Option_InfoView;
		KProgramOptionUI m_Option_RequirementsView;
		KProgramOptionUI m_Option_ComponentsView;
		KProgramOptionUI m_Option_ComponentRequirementsView;

		/* Package */
		KIWImagesMap m_ImagesMap;
		int m_CurrentImageIndex = -1;
		std::unique_ptr<KModPackage> m_Package;
		KModEntry m_ModEntry;
		const KModEntry* m_ExistingMod = NULL;

		// Components
		bool m_HasManualComponents = false;
		KIWFlagsStore m_FlagsStorage;
		KIWStepStack m_InstallSteps;
		KPPFFileEntryRefArray m_InstallableFiles;

	private:
		bool CreateUI(wxWindow* parent);
		void LoadUIOptions();

		wxWindow* CreateUI_Info();
		wxWindow* CreateUI_Requirements();
		wxWindow* CreateUI_Components();
		wxWindow* CreateUI_Installing();
		wxWindow* CreateUI_Done();

		void SetLabelByCurrentPage();

	private:
		void OpenPackage(const wxString& packagePath);
		bool LoadPackage();
		bool ProcessLoadPackage();
		void FindExistingMod();
		void AcceptExistingMod(const KModEntry& modEntry);
		void LoadHeaderImage();
		void LoadInfoList();
		void LoadMainRequirements();

		void OnNavigateImageViewer(KImageViewerEvent& event);
		void SetImageViewerNavigationInfo(KImageViewerEvent& event);
		void OnSelectDocument(int index, bool useAdvancedEditor = false);

		bool AskCancel(bool canCancel);
		void OnClose(wxCloseEvent& event);
		void OnCancelButton(wxCommandEvent& event);
		void OnGoBackward(wxCommandEvent& event);
		void OnGoForward(wxCommandEvent& event);

		void OnGoStepBackward(wxCommandEvent& event);
		void OnGoStepForward(wxCommandEvent& event);
		void OnSelectComponent(KxDataViewEvent& event);

		void StoreRequirementsFlags();
		void StoreStepFlags(const KPPCEntry::RefVector& checkedEntries);
		void RestoreStepFlagsUpToThis(const KPPCStep& step);

		bool IsConditionSatisfied(const KPPCFlagEntry& flagEntry) const;
		bool IsConditionsSatisfied(const KPPCConditionGroup& conditionGroup) const;
		bool IsStepSatisfiesConditions(const KPPCStep& step) const;
		bool CheckIsManualComponentsAvailable() const;
		KPPCStep* GetFirstStepSatisfiesConditions() const;
		KPPCStep* GetFirstStepSatisfiesConditions(const KPPCStep* afterThis) const;

		void ClearComponentsViewInfo();
		void ResetComponents();
		bool BeginComponents();
		void LoadManualStep(KPPCStep& step);
		void CollectAllInstallableEntries();
		void SortInstallableFiles();
		void ShowInstallableFilesPreview();

		bool OnBeginInstall();
		bool OnEndInstall();
		void OnMinorProgress(KxFileOperationEvent& event);
		void OnMajorProgress(KxFileOperationEvent& event);

		void SetModEntryData();
		KxUInt32Vector GetFilesOfFolder(const KPPFFolderEntry* folder) const;
		wxString GetFinalPath(uint32_t index, const wxString& installLocation, const KPPFFileEntry* fileEntry) const;
		KxStringVector GetFinalPaths(const KxUInt32Vector& filePaths, const wxString& installLocation, const KPPFFolderEntry* folder) const;
		void RunInstall();

	private:
		virtual int GetViewSizerProportion() const override
		{
			return 1;
		}
		virtual wxOrientation GetViewSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual wxOrientation GetViewLabelSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = NULL) const override
		{
			return false;
		}
		virtual bool IsCloseBoxEnabled() const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_TabView;
		}
		virtual void ResetState()
		{
		}

	public:
		KInstallWizardDialog();
		KInstallWizardDialog(wxWindow* parent, const wxString& packagePath);
		bool Create(wxWindow* parent, const wxString& packagePath);
		bool Create(wxWindow* parent, std::unique_ptr<KModPackage>&& package);
		virtual ~KInstallWizardDialog();

	public:
		const KPackageProject& GetConfig() const
		{
			return m_Package->GetConfig();
		}
		KPackageProject& GetConfig()
		{
			return m_Package->GetConfig();
		}
		const KArchive& GetArchive() const
		{
			return m_Package->GetArchive();
		}
		KArchive& GetArchive()
		{
			return m_Package->GetArchive();
		}
		const KModEntry* GetExistingMod() const
		{
			return m_ExistingMod;
		}

		const KModEntry* GetModEntry() const
		{
			return &m_ModEntry;
		}
		KModEntry* GetModEntry()
		{
			return &m_ModEntry;
		}

		bool ShouldCancel() const
		{
			return m_ShouldCancel;
		}
		bool IsCompleted() const
		{
			return m_IsComplete;
		}
		bool IsInfoPageLeft() const
		{
			return m_InfoPageLeft;
		}

		void SwitchPage(KInstallWizardPages page);
		bool OnLeavingPage(KInstallWizardPages page);

		KPPCStep* GetCurrentStep() const
		{
			return m_InstallSteps.GetTopStep();
		}
		const KIWStepStackItem* GetCurrentStepItem() const
		{
			return m_InstallSteps.GetTopItem();
		}
		bool HasMainRequirements() const;
		bool HasManualComponents() const;
		bool HasConditionalInstall() const;
};

//////////////////////////////////////////////////////////////////////////
class KIWDInstallOperation: public KOperationWithProgressBase
{
	private:
		KInstallWizardDialog* m_InstallWizard = NULL;

	public:
		KIWDInstallOperation(KInstallWizardDialog* installWizard)
			:KOperationWithProgressBase(true), m_InstallWizard(installWizard)
		{
		}

	public:
		virtual void LinkHandler(wxEvtHandler* eventHandler, wxEventType type) override
		{
			using T = wxEventTypeTag<KxFileOperationEvent>;
			GetEventHandler()->Bind(static_cast<T>(type), &KInstallWizardDialog::OnMinorProgress, m_InstallWizard);
			KOperationWithProgressBase::LinkHandler(eventHandler, type);
		}
};