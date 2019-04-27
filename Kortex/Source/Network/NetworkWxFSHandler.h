#pragma once
#include "stdafx.h"
#include <KxFramework/KxFile.h>

namespace Kortex
{
	class INetworkManager;
}

namespace Kortex::NetworkManager
{
	class NetworkWxFSHandler: public wxInternetFSHandler
	{
		private:
			INetworkManager& m_NetworkManager;

		private:
			wxString GetCacheFolder() const;
			wxString ExtractFileName(const wxString& location) const;
			wxString ConstructFullPath(const wxString& location) const;
			wxFSFile* DoOpenFile(const wxString& location) const;

			KxFile GetCachedCopyFile(const wxString& location) const;
			bool IsNewerThan(const wxDateTime& fileDate, const wxTimeSpan& span) const;

		public:
			NetworkWxFSHandler(INetworkManager& networkManager);
			virtual ~NetworkWxFSHandler();

		public:
			bool CanOpen(const wxString& location) override;
			wxString FindFirst(const wxString& wildcard, int flags) override;
			wxString FindNext() override;
			wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
	};
}