#pragma once
#include "stdafx.h"
#include "Common.h"
#include "NetworkProviderReply.h"

namespace Kortex::NetworkManager::ReplyStructs
{
	struct ModInfo
	{
		ModID ID;
		wxString Name;
		wxString m_Summary;
		wxString m_Description;
		wxString Author;
		wxString Uploader;
		wxString UploaderProfile;
		wxString MainImage;
		KxVersion Version;
		wxDateTime UploadDate;
		wxDateTime m_LastUpdateDate;
		bool ContainsAdultContent = false;
		ModEndorsement EndorsementState = ModEndorsement::Undecided();
	};
	struct ModFileInfo
	{
		using CategoryID = IModFileInfo::CategoryID;

		ModID ModID;
		ModFileID ID;
		int64_t Size = -1;
		wxString Name;
		wxString m_DisplayName;
		wxString m_ChangeLog;
		KxVersion Version;
		wxDateTime UploadDate;
		CategoryID Category = CategoryID::Unknown;
		bool IsPrimary = false;
	};
	struct ModDownloadInfo
	{
		wxString Name;
		wxString ShortName;
		wxString URL;
	};
	struct ModEndorsementInfo
	{
		ModEndorsement Endorsement = ModEndorsement::Undecided();
		wxString Message;
	};
}
