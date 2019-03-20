#include "stdafx.h"
#include "Utility/KImageProvider.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileFinder.h>

namespace KImageProvider
{
	const KxImageList* KGetImageList()
	{
		return &Kortex::IApplication::GetInstance()->GetImageList();
	}
	const KxImageSet* KGetImageSet()
	{
		return &Kortex::IApplication::GetInstance()->GetImageSet();
	}

	wxBitmap KGetBitmap(KImageEnum index)
	{
		return KGetImageList()->GetBitmap(index);
	}
	wxImage KGetImage(KImageEnum index)
	{
		return KGetImageList()->GetImage(index);
	}
	wxIcon KGetIcon(KImageEnum index)
	{
		return KGetImageList()->GetIcon(index);
	}

	wxBitmap KGetBitmap(const wxString& id)
	{
		return KGetImageSet()->GetBitmap(id);
	}
	wxImage KGetImage(const wxString& id)
	{
		return KGetImageSet()->GetImage(id);
	}
	wxIcon KGetIcon(const wxString& id)
	{
		return KGetImageSet()->GetIcon(id);
	}

	void KLoadImages(KxImageList& imageList, KxImageSet& imageSet)
	{
		wxIcon tempIcon = wxICON(IDS_ICON_APP);
		for (size_t i = 0; i < KIMG_COUNT; i++)
		{
			imageList.Add(tempIcon);
		}
		auto AddItem = [&imageList, &imageSet](KImageEnum id, const char* name, const auto& v)
		{
			imageList.Replace(id, v);
			imageSet.Set(name, v);
		};
		auto Add = [&AddItem](KImageEnum id, const char* name, int type = 0)
		{
			wxBitmap img(KxString::Format(wxS("%1\\UI\\%2.%3"), Kortex::IApplication::GetInstance()->GetDataFolder(), name, type == 0 ? wxS("png") : wxS("ico")), type == 0 ? wxBITMAP_TYPE_PNG : wxBITMAP_TYPE_ICO);
			if (img.IsOk())
			{
				AddItem(id, name, img);
			}
		};

		// Null icon
		wxImage nullIcon(wxSize(16, 16), true);
		nullIcon.InitAlpha();
		for (size_t i = 0; i < ((size_t)nullIcon.GetWidth() * (size_t)nullIcon.GetHeight()); i++)
		{
			*(nullIcon.GetAlpha() + i) = 0;
		}

		imageList.Replace(KIMG_NULL, nullIcon);
		imageSet.Set("Null", nullIcon);

		/* Site icons */
		Add(KIMG_SITE_TESALL, "TESALL");
		Add(KIMG_SITE_NEXUS, "Nexus");
		Add(KIMG_SITE_LOVERSLAB, "LoverLab");

		// Unknown icon
		KxFileItem item(wxEmptyString);
		item.SetNormalAttributes();
		item.SetName(".url");

		AddItem(KIMG_SITE_UNKNOWN, "UnknownSite", KxShell::GetFileIcon(item, true));

		// Normal icons
		Add(KIMG_7ZIP, "7zip");
		Add(KIMG_7ZIP_ICON, "7zip-icon", 1);

		Add(KIMG_MO2, "MO2");
		Add(KIMG_MO2_ICON, "MO2-icon", 1);
		
		Add(KIMG_LOOT, "LOOT");
		Add(KIMG_JSON, "JSON");
		Add(KIMG_CURL, "curl");
		Add(KIMG_WEBSOCKET, "websocket");

		Add(KIMG_PENCIL_SMALL, "pencil-small");
		Add(KIMG_PLUG_DISCONNECT, "plug-disconnect");
		Add(KIMG_PLUS, "plus");
		Add(KIMG_PLUS_SMALL, "plus-small");
		Add(KIMG_QUESTION_FRAME, "question-frame");
		Add(KIMG_TICK_CIRCLE_FRAME, "tick-circle-frame");
		Add(KIMG_TICK_CIRCLE_FRAME_EMPTY, "tick-circle-frame-empty");
		Add(KIMG_APPLICATION, "application");
		Add(KIMG_APPLICATION_DETAIL, "application-detail");
		Add(KIMG_APPLICATION_LOGO, "application-logo");
		Add(KIMG_APPLICATION_LOGO_ICON, "application-logo-icon", 1);
		Add(KIMG_APPLICATION_LOGO_SMALL, "application-logo-small");
		Add(KIMG_APPLICATION_PROHIBITION, "application-prohibition");
		Add(KIMG_APPLICATION_RUN, "application-run");
		Add(KIMG_APPLICATION_TASK, "application-task");
		Add(KIMG_SKSM_LOGO, "sksm-logo");
		Add(KIMG_SKSM_LOGO_ICON, "sksm-logo-icon", 1);
		Add(KIMG_SKSM_LOGO_SMALL, "sksm-logo-small");
		Add(KIMG_DIRECTION, "direction");
		Add(KIMG_DIRECTION_PLUS, "direction--plus");
		Add(KIMG_DIRECTION_MINUS, "direction--minus");
		Add(KIMG_BLOCK, "block");
		Add(KIMG_BLOCK_PLUS, "block--plus");
		Add(KIMG_BLOCK_MINUS, "block--minus");
		Add(KIMG_CONTROL_DOWN, "control-down");
		Add(KIMG_CONTROL_LEFT, "control-left");
		Add(KIMG_CONTROL_RIGHT, "control-right");
		Add(KIMG_CONTROL_UP, "control-up");
		Add(KIMG_CHEQUE, "cheque");
		Add(KIMG_CHEQUE_PLUS, "cheque--plus");
		Add(KIMG_CHEQUE_MINUS, "cheque--minus");
		Add(KIMG_CROSS_CIRCLE_FRAME, "cross-circle-frame");
		Add(KIMG_CROSS_CIRCLE_FRAME_EMPTY, "cross-circle-frame-empty");
		Add(KIMG_CROSS_WHITE, "cross-white");
		Add(KIMG_DISK, "disk");
		Add(KIMG_EXCLAMATION, "exclamation");
		Add(KIMG_EXCLAMATION_CIRCLE_FRAME, "exclamation-circle-frame");
		Add(KIMG_EXCLAMATION_CIRCLE_FRAME_EMPTY, "exclamation-circle-frame-empty");
		Add(KIMG_FLAG, "flag");
		Add(KIMG_FLAG_PLUS, "flag--plus");
		Add(KIMG_FLAG_MINUS, "flag--minus");
		Add(KIMG_FOLDER, "folder");
		Add(KIMG_FOLDER_OPEN, "folder-open");
		Add(KIMG_FOLDER_PLUS, "folder--plus");
		Add(KIMG_FOLDER_MINUS, "folder--minus");
		Add(KIMG_FOLDER_SEARCH_RESULT, "folder-search-result");
		Add(KIMG_FOLDER_ZIPPER, "folder-zipper");
		Add(KIMG_FOLDER_ARROW, "folder--arrow");
		Add(KIMG_FOLDERS, "folders");
		Add(KIMG_FOLDERS_PLUS, "folders--plus");
		Add(KIMG_HOME, "home");
		Add(KIMG_DOCUMENT, "document");
		Add(KIMG_DOCUMENT_NEW, "document-new");
		Add(KIMG_DOCUMENT_IMPORT, "document-import");
		Add(KIMG_DOCUMENT_EXPORT, "document-export");
		Add(KIMG_DOCUMENT_PENICL, "document--pencil");
		Add(KIMG_DOCUMENT_PLUS, "document--plus");
		Add(KIMG_DOCUMENT_MINUS, "document--minus");
		Add(KIMG_DOCUMENTS, "documents");
		Add(KIMG_DOCUMENTS_PLUS, "documents--plus");
		Add(KIMG_CALENDAR, "calendar");
		Add(KIMG_CALENDAR_DAY, "calendar-day");
		Add(KIMG_NOTIFICATION_COUNTER, "notification-counter");
		Add(KIMG_NOTIFICATION_COUNTER_42, "notification-counter-42");
		Add(KIMG_GEAR, "gear");
		Add(KIMG_GEAR_MINUS, "gear--minus");
		Add(KIMG_GEAR_PENCIL, "gear--pencil");
		Add(KIMG_GEAR_PLUS, "gear--plus");
		Add(KIMG_INFORMATION_FRAME, "information-frame");
		Add(KIMG_INFORMATION_FRAME_EMPTY, "information-frame-empty");
		Add(KIMG_JAR, "jar");
		Add(KIMG_JAR_EMPTY, "jar-empty");
		Add(KIMG_LOCALE_ALTERNATE, "locale-alternate");
		Add(KIMG_MINUS, "minus");
		Add(KIMG_MINUS_SMALL, "minus-small");
		Add(KIMG_ARROW_270, "arrow-270");
		Add(KIMG_ARROW_CIRCLE_DOUBLE, "arrow-circle-double");
		Add(KIMG_ARROW_CIRCLE_135_LEFT, "arrow-circle-135-left");
		Add(KIMG_ARROW_CURVE_180_LEFT, "arrow-curve-180-left");
		Add(KIMG_APPLICATION_DOCK_DOWN, "application-dock-down");
		Add(KIMG_APPLICATION_SIDEBAR_COLLAPSE, "application-sidebar-collapse");
		Add(KIMG_APPLICATION_SIDEBAR_EXPAND, "application-sidebar-expand");
		Add(KIMG_PUZZLE, "puzzle");
		Add(KIMG_PROJECTION_SCREEN, "projection-screen");
		Add(KIMG_CATEGORIES, "categories");
		Add(KIMG_CATEGORY, "category");
		Add(KIMG_CATEGORY_GROUP, "category-group");
		Add(KIMG_CATEGORY_GROUP_SELECT, "category-group-select");
		Add(KIMG_CATEGORY_ITEM, "category-item");
		Add(KIMG_CATEGORY_ITEM_MINUS, "category-item-minus");
		Add(KIMG_CATEGORY_ITEM_SELECT, "category-item-select");
		Add(KIMG_BOX, "box");
		Add(KIMG_BOX_MINUS, "box--minus");
		Add(KIMG_BOX_PLUS, "box--plus");
		Add(KIMG_BOX_SEARCH_RESULT, "box-search-result");
		Add(KIMG_BRIEFCASE, "briefcase");
		Add(KIMG_COMPILE, "compile");
		Add(KIMG_COMPILE_ERROR, "compile-error");
		Add(KIMG_COMPILE_WARNING, "compile-warning");
		Add(KIMG_CHART, "chart");
		Add(KIMG_ERASER, "eraser");
		Add(KIMG_TAGS, "tags");
		Add(KIMG_PICTURES, "pictures");
		Add(KIMG_IMAGE, "image");
		Add(KIMG_KEY, "key");
		Add(KIMG_LOCK, "lock");
		Add(KIMG_LOCK_SSL, "lock-ssl");
		Add(KIMG_WRENCH_SCREWDRIVER, "wrench-screwdriver");
		Add(KIMG_SORT_ALPHABET, "sort-alphabet");
		Add(KIMG_SORT_ALPHABET_DESCENDING, "sort-alphabet-descending");
		Add(KIMG_SORT_NUMBER, "sort-number");
		Add(KIMG_SORT_NUMBER_DESCENDING, "sort-number-descending");
		Add(KIMG_EDIT, "edit");
		Add(KIMG_EDIT_BOLD, "edit-bold");
		Add(KIMG_EDIT_ITALIC, "edit-italic");
		Add(KIMG_EDIT_STRIKE, "edit-strike");
		Add(KIMG_EDIT_UNDERLINE, "edit-underline");
		Add(KIMG_EDIT_CODE, "edit-code");
		Add(KIMG_EDIT_CODE_DIVISION, "edit-code-division");
		Add(KIMG_EDIT_HEADING, "edit-heading");
		Add(KIMG_EDIT_HEADING1, "edit-heading-1");
		Add(KIMG_EDIT_HEADING2, "edit-heading-2");
		Add(KIMG_EDIT_HEADING3, "edit-heading-3");
		Add(KIMG_EDIT_HEADING4, "edit-heading-4");
		Add(KIMG_EDIT_HEADING5, "edit-heading-5");
		Add(KIMG_EDIT_HEADING6, "edit-heading-6");
		Add(KIMG_EDIT_ALIGNMENT_LEFT, "edit-alignment-left");
		Add(KIMG_EDIT_ALIGNMENT_RIGHT, "edit-alignment-right");
		Add(KIMG_EDIT_ALIGNMENT_CENTER, "edit-alignment-center");
		Add(KIMG_EDIT_ALIGNMENT_JUSTIFY, "edit-alignment-justify");

		Add(KIMG_BELL, "bell");
		Add(KIMG_BELL_EXCLAMATION, "bell--exclamation");
		Add(KIMG_BELL_RED_CIRCLE, "bell--red-circle");
		Add(KIMG_BELL_PENCIL, "bell--pencil");
		Add(KIMG_BELL_PLUS, "bell--plus");
		Add(KIMG_BELL_MINUS, "bell--minus");
	}
}
