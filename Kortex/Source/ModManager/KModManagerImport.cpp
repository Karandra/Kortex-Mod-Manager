#include "stdafx.h"
#include "KModManagerImport.h"
#include "KModManagerImportMO.h"
#include "KModManagerImportNMM.h"
#include "KOperationWithProgress.h"
#include "Profile/KProfile.h"
#include "ModManager/KModWorkspace.h"
#include "DownloadManager/KDownloadWorkspace.h"
#include "GameConfig/KGameConfigWorkspace.h"
#include "ProgramManager/KProgramManagerWorkspace.h"
#include <KxFramework/KxFileBrowseDialog.h>

std::unique_ptr<KModManagerImport> KModManagerImport::Create(Type type)
{
	switch (type)
	{
		case ModOrganizer:
		{
			return std::make_unique<KModManagerImportMO>();
		}
		case NexusModManager:
		{
			return std::make_unique<KModManagerImportNMM>();
		}
	};
	return nullptr;
}
void KModManagerImport::ShowImportDialog(Type type, wxWindow* window)
{
	KxTaskDialog warningDialog(window, KxID_NONE, T("ModManager.Import.Caption"), T("ModManager.Import.OverwriteWarning"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
	if (warningDialog.ShowModal() != KxID_OK)
	{
		return;
	}

	KxFileBrowseDialog fileDialog(window, KxID_NONE, KxFBD_OPEN_FOLDER);
	if (fileDialog.ShowModal() == KxID_OK)
	{
		auto importer = KModManagerImport::Create(type);
		if (importer)
		{
			importer->SetDirectory(fileDialog.GetResult());
			if (importer->CanImport())
			{
				KxTaskDialog profileSelectDialog(window, KxID_NONE, T("ModManager.Import.Caption"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_INFO);

				profileSelectDialog.SetMessage(TF("ModManager.Import.InfoFound").arg(importer->GetModManagerName()));
				profileSelectDialog.SetExMessage(importer->GetAdditionalInfo());
				profileSelectDialog.SetCheckBoxLabel(T("ModManager.Import.SkipExisingMods"));

				profileSelectDialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED);
				profileSelectDialog.SetOptionEnabled(KxTD_CHB_ENABLED);
				profileSelectDialog.SetOptionEnabled(KxTD_CHB_CHECKED);

				KxIconType footerIcon = KxICON_NONE;
				profileSelectDialog.SetFooter(importer->GetProfileMatchingMessage(&footerIcon));
				profileSelectDialog.SetFooterIcon(footerIcon);

				// Profiles
				KxStringVector profilesList = importer->GetProfilesList();
				if (!profilesList.empty())
				{
					for (size_t i = 0; i < profilesList.size(); i++)
					{
						profileSelectDialog.AddRadioButton(i, profilesList[i]);
					}
					profileSelectDialog.SetDefaultRadioButton(0);
				}

				if (profileSelectDialog.ShowModal() == KxID_YES)
				{
					// Skip mods
					importer->SkipExistingMods(profileSelectDialog.IsCheckBoxChecked());

					wxWindowID selectedProfileIndex = profileSelectDialog.GetSelectedRadioButton();
					if (!profilesList.empty() && selectedProfileIndex != KxID_NONE)
					{
						importer->SetProfileToImport(profilesList[selectedProfileIndex]);
					}

					auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, window);
					operation->OnRun([operation, importer = importer.get()](KOperationWithProgressBase* self)
					{
						std::unique_ptr<KModManagerImport> temp(importer);
						importer->Import(operation);
					});
					operation->OnEnd([](KOperationWithProgressBase* self)
					{
						KApp::Get().CallAfter([]()
						{
							// These workspaces are safe
							KModWorkspace::GetInstance()->ScheduleReload();
							KGameConfigWorkspace::GetInstance()->ScheduleReload();
							KProgramManagerWorkspace::GetInstance()->ScheduleReload();
							KDownloadWorkspace::GetInstance()->ScheduleReload();
						});
					});
					operation->SetDialogCaption(T("ModManager.Import.Caption"));
					operation->SetOptionEnabled(KOWPD_OPTION_RUN_ONEND_BEFORE_DIALOG_DESTRUCTION);

					importer.release();
					operation->Run();
				}
				return;
			}
		}
		else
		{
			wxLogError("Invalid foreign mod-manager importer interface is requested: %d", (int)type);
		}

		KxTaskDialog errorDialog(window, KxID_NONE, T(KxID_ERROR), T("ModManager.Import.CanNotImport"), KxBTN_OK, KxICON_ERROR);
		if (errorDialog.ShowModal() != KxID_OK)
		{
			return;
		}
	}
}

wxString KModManagerImport::GetProfileMatchingMessage(KxIconType* pIcon) const
{
	KProfileID targetID = GetTargetProfileID();
	if (!targetID.IsOK())
	{
		KxUtility::SetIfNotNull(pIcon, KxICON_WARNING);
		return T("ModManager.Import.TargetProfileNotFound");
	}
	if (targetID != KProfile::GetCurrent()->GetID())
	{
		KxUtility::SetIfNotNull(pIcon, KxICON_ERROR);
		return T("ModManager.Import.TargetProfileMismatch");
	}

	KxUtility::SetIfNotNull(pIcon, KxICON_NONE);
	return wxEmptyString;
}

KModManagerImport::~KModManagerImport()
{
}
