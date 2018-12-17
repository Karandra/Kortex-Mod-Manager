#pragma once
#include "stdafx.h"
#include <KxFramework/KxImageList.h>
#include <KxFramework/KxImageSet.h>

enum KImageEnum: int
{
	KIMG_NONE = -1,
	KIMG_NULL = 0,

	KIMG_SITE_TESALL,
	KIMG_SITE_NEXUS,
	KIMG_SITE_LOVERSLAB,
	KIMG_SITE_UNKNOWN,

	KIMG_7ZIP,
	KIMG_7ZIP_ICON,

	KIMG_MO2,
	KIMG_MO2_ICON,

	KIMG_LOOT,
	KIMG_JSON,
	KIMG_CURL,
	KIMG_WEBSOCKET,

	KIMG_PENCIL_SMALL,
	KIMG_PLUG_DISCONNECT,
	KIMG_PLUS,
	KIMG_PLUS_SMALL,
	KIMG_QUESTION_FRAME,
	KIMG_TICK_CIRCLE_FRAME,
	KIMG_TICK_CIRCLE_FRAME_EMPTY,
	KIMG_APPLICATION,
	KIMG_APPLICATION_DETAIL,
	KIMG_APPLICATION_LOGO,
	KIMG_APPLICATION_LOGO_ICON,
	KIMG_APPLICATION_LOGO_SMALL,
	KIMG_APPLICATION_PROHIBITION,
	KIMG_APPLICATION_RUN,
	KIMG_APPLICATION_TASK,
	KIMG_SKSM_LOGO,
	KIMG_SKSM_LOGO_ICON,
	KIMG_SKSM_LOGO_SMALL,
	KIMG_DIRECTION,
	KIMG_DIRECTION_PLUS,
	KIMG_DIRECTION_MINUS,
	KIMG_BLOCK,
	KIMG_BLOCK_PLUS,
	KIMG_BLOCK_MINUS,
	KIMG_CONTROL_DOWN,
	KIMG_CONTROL_LEFT,
	KIMG_CONTROL_RIGHT,
	KIMG_CONTROL_UP,
	KIMG_CHEQUE,
	KIMG_CHEQUE_PLUS,
	KIMG_CHEQUE_MINUS,
	KIMG_CROSS_CIRCLE_FRAME,
	KIMG_CROSS_CIRCLE_FRAME_EMPTY,
	KIMG_CROSS_WHITE,
	KIMG_DISK,
	KIMG_EXCLAMATION,
	KIMG_EXCLAMATION_CIRCLE_FRAME,
	KIMG_EXCLAMATION_CIRCLE_FRAME_EMPTY,
	KIMG_FLAG,
	KIMG_FLAG_PLUS,
	KIMG_FLAG_MINUS,
	KIMG_FOLDER,
	KIMG_FOLDER_OPEN,
	KIMG_FOLDER_PLUS,
	KIMG_FOLDER_MINUS,
	KIMG_FOLDER_SEARCH_RESULT,
	KIMG_FOLDER_ZIPPER,
	KIMG_FOLDERS,
	KIMG_FOLDERS_PLUS,
	KIMG_DOCUMENT,
	KIMG_DOCUMENT_NEW,
	KIMG_DOCUMENT_EXPORT,
	KIMG_DOCUMENT_IMPORT,
	KIMG_DOCUMENT_PENICL,
	KIMG_DOCUMENT_PLUS,
	KIMG_DOCUMENT_MINUS,
	KIMG_DOCUMENTS,
	KIMG_DOCUMENTS_PLUS,
	KIMG_CALENDAR,
	KIMG_CALENDAR_DAY,
	KIMG_NOTIFICATION_COUNTER,
	KIMG_NOTIFICATION_COUNTER_42,
	KIMG_GEAR,
	KIMG_GEAR_MINUS,
	KIMG_GEAR_PENCIL,
	KIMG_GEAR_PLUS,
	KIMG_INFORMATION_FRAME,
	KIMG_INFORMATION_FRAME_EMPTY,
	KIMG_JAR,
	KIMG_JAR_EMPTY,
	KIMG_LOCALE_ALTERNATE,
	KIMG_MINUS,
	KIMG_MINUS_SMALL,
	KIMG_ARROW_270,
	KIMG_ARROW_CIRCLE_DOUBLE,
	KIMG_ARROW_CIRCLE_135_LEFT,
	KIMG_ARROW_CURVE_180_LEFT,
	KIMG_APPLICATION_DOCK_DOWN,
	KIMG_APPLICATION_SIDEBAR_COLLAPSE,
	KIMG_APPLICATION_SIDEBAR_EXPAND,
	KIMG_PUZZLE,
	KIMG_PROJECTION_SCREEN,
	KIMG_CATEGORIES,
	KIMG_CATEGORY,
	KIMG_CATEGORY_GROUP,
	KIMG_CATEGORY_GROUP_SELECT,
	KIMG_CATEGORY_ITEM,
	KIMG_CATEGORY_ITEM_MINUS,
	KIMG_CATEGORY_ITEM_SELECT,
	KIMG_FOLDER_ARROW,
	KIMG_HOME,
	KIMG_BOX,
	KIMG_BOX_MINUS,
	KIMG_BOX_PLUS,
	KIMG_BOX_SEARCH_RESULT,
	KIMG_BRIEFCASE,
	KIMG_COMPILE,
	KIMG_COMPILE_ERROR,
	KIMG_COMPILE_WARNING,
	KIMG_CHART,
	KIMG_ERASER,
	KIMG_TAGS,
	KIMG_PICTURES,
	KIMG_IMAGE,
	KIMG_KEY,
	KIMG_LOCK,
	KIMG_LOCK_SSL,
	KIMG_WRENCH_SCREWDRIVER,
	KIMG_SORT_ALPHABET,
	KIMG_SORT_ALPHABET_DESCENDING,
	KIMG_SORT_NUMBER,
	KIMG_SORT_NUMBER_DESCENDING,
	KIMG_EDIT,
	KIMG_EDIT_BOLD,
	KIMG_EDIT_ITALIC,
	KIMG_EDIT_UNDERLINE,
	KIMG_EDIT_STRIKE,
	KIMG_EDIT_CODE,
	KIMG_EDIT_CODE_DIVISION,
	KIMG_EDIT_HEADING,
	KIMG_EDIT_HEADING1,
	KIMG_EDIT_HEADING2,
	KIMG_EDIT_HEADING3,
	KIMG_EDIT_HEADING4,
	KIMG_EDIT_HEADING5,
	KIMG_EDIT_HEADING6,
	KIMG_EDIT_ALIGNMENT_LEFT,
	KIMG_EDIT_ALIGNMENT_RIGHT,
	KIMG_EDIT_ALIGNMENT_CENTER,
	KIMG_EDIT_ALIGNMENT_JUSTIFY,

	KIMG_BELL,
	KIMG_BELL_EXCLAMATION,
	KIMG_BELL_RED_CIRCLE,
	KIMG_BELL_PENCIL,
	KIMG_BELL_MINUS,
	KIMG_BELL_PLUS,

	KIMG_COUNT
};

namespace KImageProvider
{
	const KxImageList* KGetImageList();
	const KxImageSet* KGetImageSet();

	wxBitmap KGetBitmap(KImageEnum index);
	wxImage KGetImage(KImageEnum index);
	wxIcon KGetIcon(KImageEnum index);

	wxBitmap KGetBitmap(const wxString& id);
	wxImage KGetImage(const wxString& id);
	wxIcon KGetIcon(const wxString& id);

	void KLoadImages(KxImageList& imageList, KxImageSet& imageSet);
}

using namespace KImageProvider;
