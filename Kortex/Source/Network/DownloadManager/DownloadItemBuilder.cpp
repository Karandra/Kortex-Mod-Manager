#include "stdafx.h"
#include "DownloadItemBuilder.h"
#include "DownloadItem.h"
#include "Network/IDownloadManager.h"

namespace Kortex
{
	DownloadItemBuilder::DownloadItemBuilder()
		:m_DownloadManager(IDownloadManager::GetInstance())
	{
		m_Item = std::make_unique<DownloadItem>();
	}

	bool DownloadItemBuilder::IsOK() const
	{
		return m_Item && m_Item->IsOK();
	}
	DownloadItem* DownloadItemBuilder::Commit()
	{
		if (m_Item)
		{
			return &m_DownloadManager->AddDownload(std::move(m_Item));
		}
		return nullptr;
	}
	DownloadItem* DownloadItemBuilder::Save()
	{
		DownloadItem* item = Commit();
		if (item && !item->Save())
		{
			m_DownloadManager->RemoveDownload(*item);
			return nullptr;
		}
		return item;
	}

	DownloadItemBuilder& DownloadItemBuilder::SetTargetGame(const GameID& id)
	{
		m_Item->SetTargetGame(id);
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetModRepository(ModNetworkRepository& modRepository)
	{
		m_Item->SetModRepository(modRepository);
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetDownloadDate(const wxDateTime& date)
	{
		m_Item->m_DownloadDate = date;
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetURI(const KxURI& value)
	{
		m_Item->m_DownloadInfo.URI = value;
		return *this;
	}
	
	DownloadItemBuilder& DownloadItemBuilder::SetTotalSize(int64_t size)
	{
		m_Item->m_FileInfo.Size = size;
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetDownloadedSize(int64_t size)
	{
		m_Item->m_DownloadedSize = size;
		return *this;
	}
	
	DownloadItemBuilder& DownloadItemBuilder::SetName(const wxString& value)
	{
		m_Item->m_FileInfo.Name = value;
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetDisplayName(const wxString& value)
	{
		m_Item->m_FileInfo.DisplayName = value;
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetVersion(const KxVersion& value)
	{
		m_Item->m_FileInfo.Version = value;
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetModID(ModID modID)
	{
		m_Item->m_FileInfo.ModID = modID;
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::SetFileID(ModFileID fileID)
	{
		m_Item->m_FileInfo.ID = fileID;
		return *this;
	}
	
	DownloadItemBuilder& DownloadItemBuilder::Hide(bool value)
	{
		m_Item->Hide(value);
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::Show(bool value)
	{
		m_Item->Show(value);
		return *this;
	}
	DownloadItemBuilder& DownloadItemBuilder::ResumeFrom(int64_t pos)
	{
		m_Item->m_ShouldResume = true;
		m_Item->m_DownloadedSize = pos;

		return *this;
	}
}
