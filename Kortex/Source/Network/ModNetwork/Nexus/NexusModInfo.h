#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModRepositoryReply.h"

namespace Kortex::NetworkManager
{
	struct NexusValidationReply
	{
		wxString UserName;
		wxString APIKey;
		wxString EMailAddress;
		wxString ProfilePicture;
		int64_t UserID = -1;
		bool IsPremium = false;
		bool IsSupporter = false;
	};
	struct NexusGameReply
	{
		int64_t ID = -1;
		wxString Name;
		wxString Genre;
		wxString ForumURL;
		wxString NexusURL;
		wxString DomainName;

		int64_t FilesCount = -1;
		int64_t DownloadsCount = -1;
		int64_t ModsCount = -1;
		wxDateTime ApprovedDate;
	};
	struct NexusModFileUpdateReply
	{
		ModFileID OldID;
		ModFileID NewID;

		wxString OldName;
		wxString NewName;

		wxDateTime UploadedDate;
	};
	struct NexusNXMLinkData
	{
		wxString Key;
		wxString Expires;
		wxString UserID;

		bool IsEmpty() const
		{
			return Key.IsEmpty() && Expires.IsEmpty() && UserID.IsEmpty();
		}
	};
}
