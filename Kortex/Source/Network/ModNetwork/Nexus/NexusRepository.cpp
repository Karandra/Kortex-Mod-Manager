#include "stdafx.h"
#include "NexusRepository.h"
#include "NexusUtility.h"
#include "Nexus.h"
#include <Network/IDownloadEntry.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>

namespace Kortex::NetworkManager
{
	wxString NexusRepository::ConvertEndorsementState(const ModEndorsement& state) const
	{
		if (state.IsEndorsed())
		{
			return wxS("endorse");
		}
		if (state.IsAbstained())
		{
			return wxS("abstain");
		}
		return wxS("undecided");
	}
	bool NexusRepository::RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download)
	{
		wxString name = fileItem.GetName();
		wxRegEx reg(u8R"((.*?)\-(\d+)\-(.*)\.)", wxRE_EXTENDED|wxRE_ADVANCED|wxRE_ICASE);
		if (reg.Matches(name))
		{
			// Mod ID
			ModID modID(reg.GetMatch(name, 2));
			if (modID)
			{
				ModRepositoryRequest request(modID, {}, download.GetTargetGameID());
				for (ModFileReply& fileInfo: GetModFiles(request))
				{
					if (fileInfo.Name == name)
					{
						// Fix size discrepancy caused by Nexus sending size in kilobytes
						constexpr const int64_t oneKB = 1024 * 1024;
						const int64_t downloadedSize = download.GetDownloadedSize();
						const int64_t difference = downloadedSize - fileInfo.Size;
						if (difference > 0 && difference <= oneKB)
						{
							fileInfo.Size = downloadedSize;
						}

						download.GetFileInfo() = fileInfo;
						return true;
					}
				}
			}

			// If we got here, file is not found on Nexus, but we can try to restore as much as possible from the file name itself.
			ModFileReply& fileInfo = download.GetFileInfo();

			// Set mod ID
			fileInfo.ModID = modID;

			// Display name
			wxString displayName = reg.GetMatch(name, 1);
			displayName.Replace("_", " ");
			fileInfo.DisplayName = displayName;

			// File version
			wxString version = reg.GetMatch(name, 2);
			version.Replace("-", ".");
			fileInfo.Version = version;

			// Still return fail status
			return false;
		}
		return false;
	}

	std::optional<ModInfoReply> NexusRepository::GetModInfo(const ModRepositoryRequest& request) const
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/%3"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(request),
												 request.GetModID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply).IsSuccessful())
		{
			return std::nullopt;
		}

		ModInfoReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info.ID = request.GetModID();
			info.Name = json["name"].get<wxString>();
			info.Summary = json["summary"].get<wxString>();
			info.Description = json["description"].get<wxString>();
			info.Author = json["author"].get<wxString>();
			info.Uploader = json["uploaded_by"].get<wxString>();
			info.UploaderProfile = json["uploaded_users_profile_url"].get<wxString>();
			info.MainImage = json["picture_url"].get<wxString>();

			info.Version = json["version"].get<wxString>();
			info.UploadDate = m_Utility.ReadDateTime(json["created_time"]);
			info.LastUpdateDate = m_Utility.ReadDateTime(json["updated_time"]);

			info.ContainsAdultContent = json["contains_adult_content"];

			// Primary file
			if (auto primaryFileIt = json.find("primary_file"); primaryFileIt != json.end())
			{
				m_Utility.ReadFileInfo(*primaryFileIt, info.PrimaryFile);
			}

			// Endorsement state
			auto endorsementStateIt = json.find("endorsement");
			if (endorsementStateIt != json.end())
			{
				if (*endorsementStateIt == "Endorse")
				{
					info.EndorsementState = ModEndorsement::Endorsed();
				}
				else if (*endorsementStateIt == "Abstain")
				{
					info.EndorsementState = ModEndorsement::Abstained();
				}
				else
				{
					info.EndorsementState = ModEndorsement::Undecided();
				}
			}
		}
		catch (...)
		{
			m_Utility.ReportRequestError(reply);
			return std::nullopt;
		}
		return info;
	}
	std::optional<ModEndorsementReply> NexusRepository::EndorseMod(const ModRepositoryRequest& request, ModEndorsement state)
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/%3/%4"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(request),
												 request.GetModID().GetValue(),
												 ConvertEndorsementState(state))
		);

		KxVersion modVersion;
		if (request.GetExtraInfo(modVersion))
		{
			connection->SetPostData(KxJSON::Save(KxJSONObject {{"Version", modVersion.ToString()}}));
		}
		else
		{
			connection->SetPostData(KxJSON::Save(KxJSONObject {{"Version", "x"}}));
		}

		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply).IsSuccessful())
		{
			return std::nullopt;
		}

		ModEndorsementReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			info.Message = json["message"].get<wxString>();

			if (auto statusIt = json.find("status"); statusIt != json.end())
			{
				if (*statusIt == "Endorsed")
				{
					info.Endorsement = ModEndorsement::Endorsed();
				}
				else if (*statusIt == "Abstained")
				{
					info.Endorsement = ModEndorsement::Abstained();
				}
				else
				{
					info.Endorsement = ModEndorsement::Undecided();
				}
			}
		}
		catch (...)
		{
			m_Utility.ReportRequestError(reply);
			return std::nullopt;
		}
		return info;
	}
	
	std::optional<ModFileReply> NexusRepository::GetModFileInfo(const ModRepositoryRequest& request) const
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/%3/files/%4"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(request),
												 request.GetModID().GetValue(),
												 request.GetFileID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply).IsSuccessful())
		{
			return std::nullopt;
		}

		ModFileReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info.ModID = request.GetModID();
			m_Utility.ReadFileInfo(json, info);
		}
		catch (...)
		{
			m_Utility.ReportRequestError(reply);
			return std::nullopt;
		}
		return info;
	}
	std::vector<ModFileReply> NexusRepository::GetModFiles(const ModRepositoryRequest& request) const
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/%3/files"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(request),
												 request.GetModID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply).IsSuccessful())
		{
			return {};
		}

		std::vector<ModFileReply> infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json["files"])
			{
				ModFileReply& info = infoVector.emplace_back();
				info.ModID = request.GetModID();
				m_Utility.ReadFileInfo(value, info);
			}
		}
		catch (...)
		{
			infoVector.clear();
		}
		return infoVector;
	}
	std::vector<ModDownloadReply> NexusRepository::GetFileDownloads(const ModRepositoryRequest& request) const
	{
		wxString query = KxString::Format(wxS("%1/games/%2/mods/%3/files/%4/download_link"),
										  m_Nexus.GetAPIURL(),
										  m_Nexus.TranslateGameIDToNetwork(request),
										  request.GetModID().GetValue(),
										  request.GetFileID().GetValue()
		);

		NexusNXMLinkData nxmExtraInfo;
		if (request.GetExtraInfo(nxmExtraInfo))
		{
			query += KxString::Format("?key=%1&expires=%2", nxmExtraInfo.Key, nxmExtraInfo.Expires);
		}
		auto connection = m_Nexus.NewCURLSession(query);

		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply).IsSuccessful())
		{
			return {};
		}

		std::vector<ModDownloadReply> infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json)
			{
				ModDownloadReply& info = infoVector.emplace_back();
				info.Name = value["name"].get<wxString>();
				info.ShortName = value["short_name"].get<wxString>();

				info.URL = value["URI"].get<wxString>();
				m_Utility.ConvertUnicodeEscapes(info.URL);
			}
		}
		catch (...)
		{
			infoVector.clear();
		}
		return infoVector;
	}
}
