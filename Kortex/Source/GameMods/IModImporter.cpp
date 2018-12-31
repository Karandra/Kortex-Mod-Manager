#include "stdafx.h"
#include "IModImporter.h"
#include "GameMods/Import/ModImporterMO.h"
#include "GameMods/Import/ModImporterNMM.h"
#include "Utility/KOperationWithProgress.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/DownloadManager.hpp>
#include <Kortex/ProgramManager.hpp>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTaskDialog.h>

namespace Kortex
{
	std::unique_ptr<IModImporter> IModImporter::CreateImporter(Type type)
	{
		switch (type)
		{
			case Type::ModOrganizer:
			{
				return std::make_unique<ModManager::ModImporterMO>();
			}
			case Type::NexusModManager:
			{
				return std::make_unique<ModManager::ModImporterNMM>();
			}
		};
		return nullptr;
	}
	void IModImporter::PerformImport(Type type, wxWindow* window)
	{
		KxTaskDialog warningDialog(window, KxID_NONE, KTr("ModManager.Import.Caption"), KTr("ModManager.Import.OverwriteWarning"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
		if (warningDialog.ShowModal() != KxID_OK)
		{
			return;
		}

		KxFileBrowseDialog fileDialog(window, KxID_NONE, KxFBD_OPEN_FOLDER);
		if (fileDialog.ShowModal() == KxID_OK)
		{
			auto importer = IModImporter::CreateImporter(type);
			if (importer)
			{
				importer->SetDirectory(fileDialog.GetResult());
				if (importer->CanImport())
				{
					KxTaskDialog profileSelectDialog(window, KxID_NONE, KTr("ModManager.Import.Caption"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_INFO);

					profileSelectDialog.SetMessage(KTrf("ModManager.Import.InfoFound", importer->GetModManagerName()));
					profileSelectDialog.SetExMessage(importer->GetAdditionalInfo());
					profileSelectDialog.SetCheckBoxLabel(KTr("ModManager.Import.SkipExisingMods"));

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
							std::unique_ptr<IModImporter> temp(importer);
							importer->Import(operation);
						});
						operation->OnEnd([](KOperationWithProgressBase* self)
						{
							KWorkspace::ScheduleReloadOf<ModManager::Workspace>();
							KWorkspace::ScheduleReloadOf<DownloadManager::Workspace>();
							KWorkspace::ScheduleReloadOf<ProgramManager::Workspace>();
							//KWorkspace::ScheduleReloadOf<KGameConfigWorkspace>();
						});
						operation->SetDialogCaption(KTr("ModManager.Import.Caption"));
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

			KxTaskDialog errorDialog(window, KxID_NONE, KTr(KxID_ERROR), KTr("ModManager.Import.CanNotImport"), KxBTN_OK, KxICON_ERROR);
			if (errorDialog.ShowModal() != KxID_OK)
			{
				return;
			}
		}
	}

	wxString IModImporter::GetProfileMatchingMessage(KxIconType* pIcon) const
	{
		GameID targetID = GetTargetProfileID();
		if (!targetID.IsOK())
		{
			KxUtility::SetIfNotNull(pIcon, KxICON_WARNING);
			return KTr("ModManager.Import.TargetProfileNotFound");
		}
		if (targetID != IGameInstance::GetActive()->GetGameID())
		{
			KxUtility::SetIfNotNull(pIcon, KxICON_ERROR);
			return KTr("ModManager.Import.TargetProfileMismatch");
		}

		KxUtility::SetIfNotNull(pIcon, KxICON_NONE);
		return wxEmptyString;
	}

	IModImporter::~IModImporter()
	{
	}
}
