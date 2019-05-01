#pragma once
#include "stdafx.h"
#include "ResourceID.h"

namespace Kortex
{
	enum class ImageResourceID: int
	{
		None = -1,
		Null = 0,

		KortexLogo,
		KortexLogoIco,
		KortexLogoSmall,

		SKSMLogo,
		SKSMLogoIco,
		SKSMLogoSmall,

		ModNetwork_TESALL,
		ModNetwork_Nexus,
		ModNetwork_LoversLab,
		ModNetwork_Unknown,

		SevenZip,
		SevenZipIco,

		MO2,
		MO2Ico,

		LOOT,
		JSON,
		CURL,
		WebSocket,

		Application,
		ApplicationDetail,
		ApplicationProhibition,
		ApplicationRun,
		ApplicationTask,
		ApplicationDockDown,
		ApplicationSidebarCollapse,
		ApplicationSidebarExpand,

		Home,
		Disk,
		Tags,
		Chart,
		Image,
		Puzzle,
		Eraser,
		Pictures,
		Briefcase,
		PencilSmall,
		PlugDisconnect,
		LocaleAlternate,
		ProjectionScreen,
		WrenchScrewdriver,

		Plus,
		PlusSmall,

		Minus,
		MinusSmall,

		QuestionFrame,

		InformationFrame,
		InformationFrameEmpty,

		CrossCircleFrame,
		CrossCircleEmpty,
		CrossWhite,

		TickCircleFrame,
		TickCircleFrameEmpty,

		Exclamation,
		ExclamationCircleFrame,
		ExclamationCircleFrameEmpty,

		Direction,
		DirectionPlus,
		DirectionMinus,

		Block,
		BlockPlus,
		BlockMinus,

		ControlDown,
		ControlLeft,
		ControlRight,
		ControlUp,

		Cheque,
		ChequePlus,
		ChequeMinus,

		Flag,
		FlagPlus,
		FlagMinus,

		Folder,
		FolderOpen,
		FolderPlus,
		FolderMinus,
		FolderSearchResult,
		FolderZipper,
		FolderArrow,

		Folders,
		FoldersPlus,

		Document,
		DocumentNew,
		DocumentExport,
		DocumentImport,
		DocumentPencil,
		DocumentPlus,
		DocumentMinus,

		Documents,
		DocumentsPlus,

		Calendar,
		CalendarDay,

		NotificationCounter,
		NotificationCounter42,

		Gear,
		GearMinus,
		GearPencil,
		GearPlus,

		Jar,
		JarEmpty,

		Arrow270,
		ArrowCircleDouble,
		ArrowCircle135Left,
		ArrowCurve180Left,

		Categories,
		Category,
		CategoryMinus,
		CategoryGroup,
		CategoryGroupSelect,
		CategoryItem,
		CategoryItemSelect,

		Box,
		BoxMinus,
		BoxPlus,
		BoxSearchResult,

		Compile,
		CompileError,
		CompileWarning,

		Key,
		Lock,
		LockSSL,

		SortAlphabet,
		SortAlphabetDescending,
		SorNumber,
		SorNumberDescending,

		Edit,
		EditBold,
		EditItalic,
		EditUnderline,
		EditStrike,

		EditCode,
		EditCodeDivision,

		EditHeading,
		EditHeading1,
		EditHeading2,
		EditHeading3,
		EditHeading4,
		EditHeading5,
		EditHeading6,

		EditAlignmentLeft,
		EditAlignmentRight,
		EditAlignmentCenter,
		EditAlignmentJustify,

		Bell,
		BellExclamation,
		BellRedCircle,
		BellPencil,
		BellMinus,
		BellPlus,

		MAX_ELEMENT
	};
}
