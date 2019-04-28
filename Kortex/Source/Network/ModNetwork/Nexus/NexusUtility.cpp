#include "stdafx.h"
#include "NexusUtility.h"
#include "Nexus.h"
#include "Network/INetworkManager.h"
#include "Application/INotificationCenter.h"
#include <KxFramework/KxIndexedEnum.h>
#include <KxFramework/KxCURL.h>

namespace
{
	using Kortex::ModFileCategory;

	struct CategoryDef: public KxIndexedEnum::Definition<CategoryDef, ModFileCategory, wxString>
	{
		inline static const TItem ms_Index[] =
		{
			{ModFileCategory::Main, wxS("MAIN")},
			{ModFileCategory::Optional, wxS("OPTIONAL")},
		};
	};
}

namespace Kortex::NetworkManager
{
	void NexusUtility::ConvertChangeLog(wxString& changeLog) const
	{
		changeLog.Replace(wxS("<br>"), wxS("\r\n"));
		changeLog.Replace(wxS("<br/>"), wxS("\r\n"));
		changeLog.Replace(wxS("<br />"), wxS("\r\n"));
		changeLog.Replace(wxS("</br>"), wxS("\r\n"));

		changeLog.Replace(wxS("\n\r\n"), wxS("\r\n"));
		KxString::Trim(changeLog, true, true);
	}
	void NexusUtility::ConvertDisplayName(wxString& name) const
	{
		name.Replace(wxS("_"), wxS(" "));
		KxString::Trim(name, true, true);
	}
	void NexusUtility::ConvertUnicodeEscapes(wxString& source) const
	{
		// Find and replace all '\uABCD' 6-char hex patterns to corresponding Unicode codes.
		// This is almost the same as 'ModImporterMO::DecodeUTF8'. Need to generalize and merge these functions.
		constexpr size_t prefixLength = 2;
		constexpr size_t sequenceLength = 6;
		constexpr size_t valueLength = sequenceLength - prefixLength;

		for (size_t i = 0; i < source.Length(); i++)
		{
			size_t pos = source.find(wxS("\\u"), i);
			if (pos != wxString::npos)
			{
				unsigned long long value = 0;
				if (source.Mid(pos + prefixLength, valueLength).ToULongLong(&value, 16) && value != 0)
				{
					wxUniChar c(value);
					source.replace(pos, sequenceLength, c);
				}
			}
		}
	}

	wxDateTime NexusUtility::ReadDateTime(const TJsonValue& json) const
	{
		wxDateTime date;
		date.ParseISOCombined(json.get<wxString>());
		return date.FromUTC(date.IsDST());
	}
	void NexusUtility::ReadFileInfo(const TJsonValue& json, ModFileReply& info) const
	{
		info.ID = json["file_id"].get<ModID::TValue>();
		info.IsPrimary = json["is_primary"];
		info.Name = json["file_name"].get<wxString>();
		info.DisplayName = json["name"].get<wxString>();
		info.Version = json["version"].get<wxString>();
		info.ChangeLog = json["changelog_html"].get<wxString>();
		info.UploadDate = ReadDateTime(json["uploaded_time"]);

		ConvertDisplayName(info.DisplayName);
		ConvertChangeLog(info.ChangeLog);

		// WTF?! Why file size is in kilobytes instead of bytes?
		// Ok, I convert it here, though final size may be a bit smaller.
		// At least download manager can request correct file size upon downloading.
		info.Size = json["size"].get<int64_t>() * 1024;

		// Values: 'MAIN', 'OPTIONAL', <TBD>.
		info.Category = CategoryDef::FromString(json["category_name"].get<wxString>(), ModFileCategory::Unknown);
	}
	void NexusUtility::ReadGameInfo(const TJsonValue& json, NexusGameReply& info) const
	{
		info.ID = json["id"];
		info.Name = json["name"].get<wxString>();
		info.Genre = json["genre"].get<wxString>();
		info.ForumURL = json["forum_url"].get<wxString>();
		info.NexusURL = json["nexusmods_url"].get<wxString>();
		info.DomainName = json["domain_name"].get<wxString>();

		info.FilesCount = json["file_count"];
		info.DownloadsCount = json["downloads"];
		info.ModsCount = json["mods"];
		info.ApprovedDate = wxDateTime((time_t)json["approved_date"]);
	}

	void NexusUtility::ReportRequestError(const wxString& message) const
	{
		try
		{
			KxJSONObject json = KxJSON::Load(message);
			INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(json["message"].get<wxString>(), KxICON_ERROR);
		}
		catch (...)
		{
			INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(message, KxICON_ERROR);
		}
	}
	bool NexusUtility::TestRequestError(const KxCURLReplyBase& reply, const wxString& message, bool noErrorReport) const
	{
		if (reply.GetResponseCode() == KxHTTPStatusCode::TooManyRequests)
		{
			if (!noErrorReport)
			{
				INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(KTrf("NetworkManager.RequestQuotaReched",
																				 NexusModNetwork::GetInstance()->GetName()),
																				 KxICON_WARNING);
			}
			return true;
		}
		else if (!reply.IsOK())
		{
			if (!noErrorReport)
			{
				ReportRequestError(message);
			}
			return true;
		}
		return false;
	}
}
