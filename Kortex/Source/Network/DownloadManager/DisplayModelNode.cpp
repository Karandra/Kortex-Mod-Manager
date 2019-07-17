#include "stdafx.h"
#include "DisplayModelNode.h"
#include "DownloadItem.h"
#include "Utility/KAux.h"
#include <Kortex/Resources.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/NetworkManager.hpp>
#include <KxFramework/KxComparator.h>

namespace Kortex::DownloadManager
{
	wxAny DisplayModelNode::GetValue(const KxDataView2::Column& column) const
	{
		using namespace KxDataView2;

		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				return BitmapTextValue(m_Item.GetName(), GetStateBitmap());
			}
			case ColumnID::Version:
			{
				return m_Item.GetVersion().ToString();
			}
			case ColumnID::Size:
			{
				const int64_t size = m_Item.GetTotalSize();
				return size >= 0 ? KxFile::FormatFileSize(size, 2) : wxEmptyString;
			}
			case ColumnID::Game:
			{
				if (GameID gameID = m_Item.GetTargetGame())
				{
					return gameID->GetGameShortName();
				}
				break;
			}
			case ColumnID::Source:
			{
				wxString label;
				if (IModNetwork* modNetwork = m_Item.GetModNetwork())
				{
					label = modNetwork->GetName();
				}
				if (wxString server = m_Item.GetServerName(); !server.IsEmpty())
				{
					if (!label.IsEmpty())
					{
						if (server.Contains(wxS("(")) || server.Contains(wxS(")")))
						{
							label += KxString::Format(wxS(" [%1]"), server);
						}
						else
						{
							label += KxString::Format(wxS(" (%1)"), server);
						}
					}
					else
					{
						label = server;
					}
				}
				return label;
			}
			case ColumnID::Date:
			{
				return KAux::FormatDateTime(m_Item.GetDownloadDate());
			}
			case ColumnID::Status:
			{
				// Percent
				const int64_t downloadedSize = m_Item.GetDownloadedSize();
				const int64_t totalSize = m_Item.GetTotalSize();
				int percent = 0;
				if (totalSize > 0)
				{
					percent = ((double)downloadedSize / (double)totalSize) * 100;
				}

				// Bar color
				ProgressState state = ProgressState::Normal;
				if (m_Item.IsPaused())
				{
					state = ProgressState::Paused;
				}
				else if (m_Item.IsFailed())
				{
					state = ProgressState::Error;
				}

				// Label
				wxString label = KxString::Format(wxS("%1%"), percent);
				if (m_Item.IsRunning())
				{
					// Add speed per sec
					label += KxString::Format(wxS(", %1/%2"), KxFile::FormatFileSize(m_Item.GetDownloadSpeed(), 0), KTr(wxS("Generic.Sec")));
				}

				bool addedDownloaded = false;
				if (!m_Item.IsCompleted() && downloadedSize >= 0)
				{
					// Add downloaded bytes so far
					addedDownloaded = true;

					label += wxS(", ");
					label += KxFile::FormatFileSize(downloadedSize, 2);
				}
				if (!m_SizeColumn->IsVisible() && totalSize >= 0)
				{
					// Add total size if size column is hidden
					label += addedDownloaded ? wxS("/") : wxS(", ");
					label += KxFile::FormatFileSize(totalSize, 2);
				}

				return ProgressValue(percent, label, state);;
			}
		};
		return {};
	}
	bool DisplayModelNode::SetValue(KxDataView2::Column& column, const wxAny& value)
	{
		return false;
	}
	KxDataView2::ToolTip DisplayModelNode::GetToolTip(const KxDataView2::Column& column) const
	{
		return KxDataView2::Node::GetToolTip(column);
	}

	bool DisplayModelNode::IsEnabled(const KxDataView2::Column& column) const
	{
		return !m_Item.IsRunning();
	}
	bool DisplayModelNode::Compare(const KxDataView2::Node& otherNode, const KxDataView2::Column& column) const
	{
		const DownloadItem& other = static_cast<const DisplayModelNode&>(otherNode).m_Item;

		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				return KxComparator::IsLess(m_Item.GetName(), other.GetName());
			}
			case ColumnID::Version:
			{
				return m_Item.GetVersion() < other.GetVersion();
			}
			case ColumnID::Size:
			{
				return m_Item.GetTotalSize() < other.GetTotalSize();
			}
			case ColumnID::Game:
			{
				const GameID left = m_Item.GetTargetGame();
				const GameID right = other.GetTargetGame();

				return KxComparator::IsLess(left ? left->GetGameShortName() : wxEmptyString,
											right ? right->GetGameShortName() : wxEmptyString);
			}
			case ColumnID::Source:
			{
				const IModNetwork* left = m_Item.GetModNetwork();
				const IModNetwork* right = other.GetModNetwork();

				return KxComparator::IsLess(left ? left->GetName() : wxEmptyString,
											right ? right->GetName() : wxEmptyString);
			}
			case ColumnID::Date:
			{
				return m_Item.GetDownloadDate() < other.GetDownloadDate();
			}
		};
		return false;
	}

	wxBitmap DisplayModelNode::GetStateBitmap() const
	{
		if (!m_Item.IsOK())
		{
			return ImageProvider::GetBitmap(ImageResourceID::ExclamationCircleFrame);
		}
		if (m_Item.IsCompleted())
		{
			return ImageProvider::GetBitmap(ImageResourceID::TickCircleFrame);
		}
		if (m_Item.IsFailed())
		{
			return ImageProvider::GetBitmap(ImageResourceID::CrossCircleFrame);
		}
		if (m_Item.IsPaused())
		{
			return ImageProvider::GetBitmap(ImageResourceID::ExclamationCircleFrameEmpty);
		}
		return ImageProvider::GetBitmap(ImageResourceID::TickCircleFrameEmpty);
	}
}
