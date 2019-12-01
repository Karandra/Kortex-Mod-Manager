#include "stdafx.h"
#include "DefaultImageProvider.h"
#include "ImageResourceID.h"
#include "Utility/KBitmapSize.h"
#include <KxFramework/KxFileItem.h>
#include <KxFramework/KxShell.h>

namespace
{
	wxSize GetIconSIze()
	{
		return KBitmapSize().FromSystemSmallIcon();
	}
	int GetIconCount()
	{
		return static_cast<int>(Kortex::ImageResourceID::MAX_ELEMENT);
	}

	bool operator<(const wxSize& left, const wxSize& right)
	{
		return left.GetWidth() < right.GetWidth() || left.GetHeight() < right.GetHeight();
	}
}

namespace Kortex::Application
{
	void DefaultImageProvider::OnLoadBitmap(wxBitmap& bitmap)
	{
		if (bitmap.GetSize() < m_ImageList.GetSize())
		{
			wxSize targetSize = m_ImageList.GetSize();
			wxImage image = bitmap.ConvertToImage();
			image.Rescale(targetSize.GetWidth(), targetSize.GetHeight(), wxImageResizeQuality::wxIMAGE_QUALITY_NORMAL);
			bitmap = wxBitmap(image, 32);
		}
	}
	void DefaultImageProvider::LoadImages()
	{
		// Initialize image list with dummy images
		{
			wxIcon tempIcon = wxICON(IDS_ICON_APP);
			for (size_t i = 0; i < static_cast<int>(ImageResourceID::MAX_ELEMENT); i++)
			{
				m_ImageList.Add(tempIcon);
			}
		}

		// Add empty transparent icon
		if (wxImage nullIcon(GetIconSIze(), true); true)
		{
			nullIcon.InitAlpha();
			memset(nullIcon.GetAlpha(), 0, (size_t)nullIcon.GetWidth() * (size_t)nullIcon.GetHeight());

			AddSingleItem(ImageResourceID::Null, wxS("Null"), nullIcon);
		}

		// Unknown icon
		if (KxFileItem item(wxEmptyString); true)
		{
			item.SetNormalAttributes();
			item.SetName(wxS(".url"));

			AddSingleItem(ImageResourceID::ModNetwork_Unknown, wxS("UnknownSite"), KxShell::GetFileIcon(item, true));
		}

		// App logos
		LoadItem(ImageResourceID::KortexLogo, wxS("kortex-logo"));
		LoadItem(ImageResourceID::KortexLogoIco, wxS("kortex-logo-icon"), Type::Icon);
		LoadItem(ImageResourceID::KortexLogoSmall, wxS("kortex-logo-small"));

		LoadItem(ImageResourceID::SKSMLogo, wxS("sksm-logo"));
		LoadItem(ImageResourceID::SKSMLogoIco, wxS("sksm-logo-icon"), Type::Icon);
		LoadItem(ImageResourceID::SKSMLogoSmall, wxS("sksm-logo-small"));

		// Mod networks icons
		LoadItem(ImageResourceID::ModNetwork_TESALL, wxS("TESALL.RU"));
		LoadItem(ImageResourceID::ModNetwork_Nexus, wxS("NexusMods"));
		LoadItem(ImageResourceID::ModNetwork_LoversLab, wxS("LoverLab"));

		// Normal icons
		LoadItem(ImageResourceID::SevenZip, wxS("7zip"));
		LoadItem(ImageResourceID::SevenZipIco, wxS("7zip-icon"), Type::Icon);

		LoadItem(ImageResourceID::MO2, wxS("MO2"));
		LoadItem(ImageResourceID::MO2Ico, wxS("MO2-icon"), Type::Icon);

		LoadItem(ImageResourceID::LOOT, wxS("LOOT"));
		LoadItem(ImageResourceID::JSON, wxS("JSON"));
		LoadItem(ImageResourceID::Dokany, wxS("Dokany"));
		LoadItem(ImageResourceID::LibCURL, wxS("curl"));
		LoadItem(ImageResourceID::WebSocket, wxS("websocket"));

		LoadItem(ImageResourceID::Application, wxS("application"));
		LoadItem(ImageResourceID::ApplicationDetail, wxS("application-detail"));
		LoadItem(ImageResourceID::ApplicationProhibition, wxS("application-prohibition"));
		LoadItem(ImageResourceID::ApplicationRun, wxS("application-run"));
		LoadItem(ImageResourceID::ApplicationTask, wxS("application-task"));
		LoadItem(ImageResourceID::ApplicationDockDown, wxS("application-dock-down"));
		LoadItem(ImageResourceID::ApplicationSidebarCollapse, wxS("application-sidebar-collapse"));
		LoadItem(ImageResourceID::ApplicationSidebarExpand, wxS("application-sidebar-expand"));

		LoadItem(ImageResourceID::Home, wxS("home"));
		LoadItem(ImageResourceID::Disk, wxS("disk"));
		LoadItem(ImageResourceID::Tags, wxS("tags"));
		LoadItem(ImageResourceID::Chart, wxS("chart"));
		LoadItem(ImageResourceID::Image, wxS("image"));
		LoadItem(ImageResourceID::Puzzle, wxS("puzzle"));
		LoadItem(ImageResourceID::Eraser, wxS("eraser"));
		LoadItem(ImageResourceID::Pictures, wxS("pictures"));
		LoadItem(ImageResourceID::Briefcase, wxS("briefcase"));
		LoadItem(ImageResourceID::PencilSmall, wxS("pencil-small"));
		LoadItem(ImageResourceID::PlugDisconnect, wxS("plug-disconnect"));
		LoadItem(ImageResourceID::LocaleAlternate, wxS("locale-alternate"));
		LoadItem(ImageResourceID::ProjectionScreen, wxS("projection-screen"));
		LoadItem(ImageResourceID::WrenchScrewdriver, wxS("wrench-screwdriver"));

		LoadItem(ImageResourceID::Plus, wxS("plus"));
		LoadItem(ImageResourceID::PlusSmall, wxS("plus-small"));

		LoadItem(ImageResourceID::Minus, wxS("minus"));
		LoadItem(ImageResourceID::MinusSmall, wxS("minus-small"));

		LoadItem(ImageResourceID::QuestionFrame, wxS("question-frame"));

		LoadItem(ImageResourceID::InformationFrame, wxS("information-frame"));
		LoadItem(ImageResourceID::InformationFrameEmpty, wxS("information-frame-empty"));

		LoadItem(ImageResourceID::CrossCircleFrame, wxS("cross-circle-frame"));
		LoadItem(ImageResourceID::CrossCircleEmpty, wxS("cross-circle-frame-empty"));
		LoadItem(ImageResourceID::CrossWhite, wxS("cross-white"));

		LoadItem(ImageResourceID::TickCircleFrame, wxS("tick-circle-frame"));
		LoadItem(ImageResourceID::TickCircleFrameEmpty, wxS("tick-circle-frame-empty"));

		LoadItem(ImageResourceID::Exclamation, wxS("exclamation"));
		LoadItem(ImageResourceID::ExclamationCircleFrame, wxS("exclamation-circle-frame"));
		LoadItem(ImageResourceID::ExclamationCircleFrameEmpty, wxS("exclamation-circle-frame-empty"));

		LoadItem(ImageResourceID::Direction, wxS("direction"));
		LoadItem(ImageResourceID::DirectionPlus, wxS("direction--plus"));
		LoadItem(ImageResourceID::DirectionMinus, wxS("direction--minus"));

		LoadItem(ImageResourceID::Block, wxS("block"));
		LoadItem(ImageResourceID::BlockPlus, wxS("block--plus"));
		LoadItem(ImageResourceID::BlockMinus, wxS("block--minus"));

		LoadItem(ImageResourceID::ControlCursor, wxS("control-cursor"));
		LoadItem(ImageResourceID::ControlPause, wxS("control-pause"));
		LoadItem(ImageResourceID::ControlStop, wxS("control-stop"));
		LoadItem(ImageResourceID::ControlStopSquare, wxS("control-stop-square"));
		LoadItem(ImageResourceID::ControlDown, wxS("control-down"));
		LoadItem(ImageResourceID::ControlLeft, wxS("control-left"));
		LoadItem(ImageResourceID::ControlRight, wxS("control-right"));
		LoadItem(ImageResourceID::ControlUp, wxS("control-up"));

		LoadItem(ImageResourceID::Cheque, wxS("cheque"));
		LoadItem(ImageResourceID::ChequePlus, wxS("cheque--plus"));
		LoadItem(ImageResourceID::ChequeMinus, wxS("cheque--minus"));

		LoadItem(ImageResourceID::Flag, wxS("flag"));
		LoadItem(ImageResourceID::FlagPlus, wxS("flag--plus"));
		LoadItem(ImageResourceID::FlagMinus, wxS("flag--minus"));

		LoadItem(ImageResourceID::Folder, wxS("folder"));
		LoadItem(ImageResourceID::FolderOpen, wxS("folder-open"));
		LoadItem(ImageResourceID::FolderPlus, wxS("folder--plus"));
		LoadItem(ImageResourceID::FolderMinus, wxS("folder--minus"));
		LoadItem(ImageResourceID::FolderSearchResult, wxS("folder-search-result"));
		LoadItem(ImageResourceID::FolderZipper, wxS("folder-zipper"));
		LoadItem(ImageResourceID::FolderArrow, wxS("folder--arrow"));

		LoadItem(ImageResourceID::Folders, wxS("folders"));
		LoadItem(ImageResourceID::FoldersPlus, wxS("folders--plus"));
		
		LoadItem(ImageResourceID::Document, wxS("document"));
		LoadItem(ImageResourceID::DocumentNew, wxS("document-new"));
		LoadItem(ImageResourceID::DocumentImport, wxS("document-import"));
		LoadItem(ImageResourceID::DocumentExport, wxS("document-export"));
		LoadItem(ImageResourceID::DocumentPencil, wxS("document--pencil"));
		LoadItem(ImageResourceID::DocumentPlus, wxS("document--plus"));
		LoadItem(ImageResourceID::DocumentMinus, wxS("document--minus"));

		LoadItem(ImageResourceID::Documents, wxS("documents"));
		LoadItem(ImageResourceID::DocumentsPlus, wxS("documents--plus"));

		LoadItem(ImageResourceID::Calendar, wxS("calendar"));
		LoadItem(ImageResourceID::CalendarDay, wxS("calendar-day"));

		LoadItem(ImageResourceID::NotificationCounter, wxS("notification-counter"));
		LoadItem(ImageResourceID::NotificationCounter42, wxS("notification-counter-42"));

		LoadItem(ImageResourceID::Gear, wxS("gear"));
		LoadItem(ImageResourceID::GearMinus, wxS("gear--minus"));
		LoadItem(ImageResourceID::GearPencil, wxS("gear--pencil"));
		LoadItem(ImageResourceID::GearPlus, wxS("gear--plus"));
		
		LoadItem(ImageResourceID::Jar, wxS("jar"));
		LoadItem(ImageResourceID::JarEmpty, wxS("jar-empty"));
		
		LoadItem(ImageResourceID::Arrow270, wxS("arrow-270"));
		LoadItem(ImageResourceID::ArrowCircleDouble, wxS("arrow-circle-double"));
		LoadItem(ImageResourceID::ArrowCircle135Left, wxS("arrow-circle-135-left"));
		LoadItem(ImageResourceID::ArrowCurve180Left, wxS("arrow-curve-180-left"));
		
		LoadItem(ImageResourceID::Categories, wxS("categories"));
		LoadItem(ImageResourceID::Category, wxS("category"));
		LoadItem(ImageResourceID::CategoryGroup, wxS("category-group"));
		LoadItem(ImageResourceID::CategoryGroupSelect, wxS("category-group-select"));
		LoadItem(ImageResourceID::CategoryItem, wxS("category-item"));
		LoadItem(ImageResourceID::CategoryMinus, wxS("category-item-minus"));
		LoadItem(ImageResourceID::CategoryItemSelect, wxS("category-item-select"));

		LoadItem(ImageResourceID::Box, wxS("box"));
		LoadItem(ImageResourceID::BoxMinus, wxS("box--minus"));
		LoadItem(ImageResourceID::BoxPlus, wxS("box--plus"));
		LoadItem(ImageResourceID::BoxSearchResult, wxS("box-search-result"));
		
		LoadItem(ImageResourceID::Compile, wxS("compile"));
		LoadItem(ImageResourceID::CompileError, wxS("compile-error"));
		LoadItem(ImageResourceID::CompileWarning, wxS("compile-warning"));

		LoadItem(ImageResourceID::Key, wxS("key"));
		LoadItem(ImageResourceID::Lock, wxS("lock"));
		LoadItem(ImageResourceID::LockSSL, wxS("lock-ssl"));
		
		LoadItem(ImageResourceID::Magnifier, wxS("magnifier"));
		LoadItem(ImageResourceID::MagnifierPlus, wxS("magnifier--plus"));
		LoadItem(ImageResourceID::MagnifierMinus, wxS("magnifier--minus"));
		LoadItem(ImageResourceID::MagnifierPencil, wxS("magnifier--pencil"));
		LoadItem(ImageResourceID::MagnifierLeft, wxS("magnifier-left"));
		LoadItem(ImageResourceID::MagnifierZoom, wxS("magnifier-zoom"));
		LoadItem(ImageResourceID::MagnifierZoomIn, wxS("magnifier-zoom-in"));
		LoadItem(ImageResourceID::MagnifierZoomOut, wxS("magnifier-zoom-out"));
		
		LoadItem(ImageResourceID::SortAlphabet, wxS("sort-alphabet"));
		LoadItem(ImageResourceID::SortAlphabetDescending, wxS("sort-alphabet-descending"));
		LoadItem(ImageResourceID::SorNumber, wxS("sort-number"));
		LoadItem(ImageResourceID::SorNumberDescending, wxS("sort-number-descending"));

		LoadItem(ImageResourceID::Edit, wxS("edit"));
		LoadItem(ImageResourceID::EditBold, wxS("edit-bold"));
		LoadItem(ImageResourceID::EditItalic, wxS("edit-italic"));
		LoadItem(ImageResourceID::EditStrike, wxS("edit-strike"));
		LoadItem(ImageResourceID::EditUnderline, wxS("edit-underline"));

		LoadItem(ImageResourceID::EditCode, wxS("edit-code"));
		LoadItem(ImageResourceID::EditCodeDivision, wxS("edit-code-division"));

		LoadItem(ImageResourceID::EditHeading, wxS("edit-heading"));
		LoadItem(ImageResourceID::EditHeading1, wxS("edit-heading-1"));
		LoadItem(ImageResourceID::EditHeading2, wxS("edit-heading-2"));
		LoadItem(ImageResourceID::EditHeading3, wxS("edit-heading-3"));
		LoadItem(ImageResourceID::EditHeading4, wxS("edit-heading-4"));
		LoadItem(ImageResourceID::EditHeading5, wxS("edit-heading-5"));
		LoadItem(ImageResourceID::EditHeading6, wxS("edit-heading-6"));

		LoadItem(ImageResourceID::EditAlignmentLeft, wxS("edit-alignment-left"));
		LoadItem(ImageResourceID::EditAlignmentRight, wxS("edit-alignment-right"));
		LoadItem(ImageResourceID::EditAlignmentCenter, wxS("edit-alignment-center"));
		LoadItem(ImageResourceID::EditAlignmentJustify, wxS("edit-alignment-justify"));

		LoadItem(ImageResourceID::EditList, wxS("edit-list"));
		LoadItem(ImageResourceID::EditListOrder, wxS("edit-list-order"));

		LoadItem(ImageResourceID::Bell, wxS("bell"));
		LoadItem(ImageResourceID::BellExclamation, wxS("bell--exclamation"));
		LoadItem(ImageResourceID::BellRedCircle, wxS("bell--red-circle"));
		LoadItem(ImageResourceID::BellPencil, wxS("bell--pencil"));
		LoadItem(ImageResourceID::BellPlus, wxS("bell--plus"));
		LoadItem(ImageResourceID::BellMinus, wxS("bell--minus"));

		LoadItem(ImageResourceID::Bin, wxS("bin"));
		LoadItem(ImageResourceID::BinArrow, wxS("bin--arrow"));
		LoadItem(ImageResourceID::BinFull, wxS("bin-full"));

		LoadItem(ImageResourceID::Broom, wxS("broom"));
	}

	DefaultImageProvider::DefaultImageProvider()
		:m_ImageList(GetIconSIze(), GetIconCount()), m_ImageSet(GetIconCount())
	{
	}
}
