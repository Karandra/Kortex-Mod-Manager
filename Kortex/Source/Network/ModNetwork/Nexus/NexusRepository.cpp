#include "stdafx.h"
#include "NexusRepository.h"
#include "NexusUtility.h"
#include "Nexus.h"
#include "NXMHandler/Dialog.h"
#include "NXMHandler/OptionStore.h"
#include "Application/Options/CmdLineDatabase.h"
#include <Kortex/Application.hpp>
#include <Kortex/DownloadManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "Utility/Common.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxMenu.h>

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
	void NexusRepository::OnResponseHeader(KxCURLEvent& event)
	{
		event.Skip();
		const wxString headerName = event.GetHeaderName();

		auto ToInt = [&event]() -> std::optional<int>
		{
			if (long intValue = -1; event.GetHeaderValue().ToCLong(&intValue))
			{
				return intValue;
			}
			return std::nullopt;
		};
		auto TestInt = [&headerName, &ToInt](const wxChar* name, int& ref)
		{
			if (headerName == name)
			{
				if (auto value = ToInt())
				{
					ref = *value;
				}
			}
		};
		auto TestISODate = [&headerName, &event](const wxChar* name, wxDateTime& ref)
		{
			if (headerName == name)
			{
				ref.ParseISOCombined(event.GetHeaderValue());
			}
		};

		if (wxCriticalSectionLocker lock(m_LimitsDataCS); true)
		{
			TestInt(wxS("X-RL-Hourly-Limit"), m_LimitsData.HourlyTotal);
			TestInt(wxS("X-RL-Hourly-Remaining"), m_LimitsData.HourlyRemaining);
			TestISODate(wxS("X-RL-Hourly-Reset"), m_LimitsData.HourlyLimitReset);

			TestInt(wxS("X-RL-Daily-Limit"), m_LimitsData.DailyTotal);
			TestInt(wxS("X-RL-Daily-Remaining"), m_LimitsData.DailyRemaining);
			TestISODate(wxS("X-RL-Reset-Reset"), m_LimitsData.DailyLimitReset);
		}
	}
	
	AppOption NexusRepository::GetNXMHandlerOptions() const
	{
		return Application::GetGlobalOptionOf<INetworkManager>(m_Nexus.GetName(), wxS("NXMHandler"));
	}
	void NexusRepository::LoadNXMHandlerOptions()
	{
		if (!m_NXMHandlerOptions)
		{
			m_NXMHandlerOptions = std::make_unique<NXMHandler::OptionStore>();
			m_NXMHandlerOptions->Load(GetNXMHandlerOptions());
		}
	}
	
	NexusRepository::NexusRepository(NexusModNetwork& nexus, NexusUtility& utility, NexusAuth& auth)
		:m_Nexus(nexus), m_Utility(utility), m_Auth(auth)
	{
	}
	NexusRepository::~NexusRepository()
	{
	}

	ModRepositoryLimits NexusRepository::GetRequestLimits() const
	{
		wxCriticalSectionLocker lock(m_LimitsDataCS);
		return m_LimitsData;
	}
	bool NexusRepository::IsAutomaticUpdateCheckAllowed() const
	{
		wxCriticalSectionLocker lock(m_LimitsDataCS);

		// Allow only if:
		// - We are authenticated on Nexus.
		// - Remaining daily limit is greater than 10% of total limit.

		auto CheckDaily = [this](double percent)
		{
			return m_LimitsData.DailyRemaining > m_LimitsData.DailyTotal * percent;
		};
		return m_Auth.IsAuthenticated() && CheckDaily(0.1);
	}
	bool NexusRepository::ParseDownloadName(const wxString& name, ModFileReply& result)
	{
		wxRegEx reg(u8R"((.+?)\-(\d+)\-(.+?\-?.*?)\-(.+)\.)", wxRE_ADVANCED|wxRE_ICASE);
		if (reg.Matches(name))
		{
			result.Name = reg.GetMatch(name, 1);
			result.ModID = reg.GetMatch(name, 2);
			result.ID = reg.GetMatch(name, 4);

			wxString version = reg.GetMatch(name, 3);
			version.Replace(wxS("-"), wxS("."));
			result.Version = version;

			if (result.DisplayName.IsEmpty())
			{
				result.DisplayName = result.Name;
				result.DisplayName.Replace("_", " ");
			}

			return !result.Name.IsEmpty() && result.ModID && result.ID && result.Version.IsOK();
		}
		return false;
	}

	bool NexusRepository::QueryDownload(const KxFileItem& fileItem, const DownloadItem& download, ModFileReply& fileReply)
	{
		auto QueryInfo = [this, &download](ModID modID, ModFileID fileID) -> std::optional<ModFileReply>
		{
			ModRepositoryRequest request(modID, fileID, download.GetTargetGame());
			auto fileReply = GetModFileInfo(request);
			if (fileReply)
			{
				// Fix size discrepancy caused by Nexus sending size in kilobytes
				constexpr const int64_t oneKB = 1024 * 1024;
				const int64_t downloadedSize = download.GetDownloadedSize();
				const int64_t difference = downloadedSize - fileReply->Size;
				if (difference > 0 && difference <= oneKB)
				{
					fileReply->Size = downloadedSize;
				}

				return fileReply;
			}
			return std::nullopt;
		};

		// If the download contains no usable info, parse file name to try to find mod and file IDs
		bool nameParsed = false;
		if (!download.IsOK())
		{
			nameParsed = ParseDownloadName(fileItem.GetName(), fileReply);
			// Don't return here if the name wasn't parsed successfully. The download can still contain mod and/or file ID which we can use.
		}

		// Query new mod file info
		if (auto newFileReply = QueryInfo(fileReply.ModID, fileReply.ID))
		{
			fileReply = std::move(*newFileReply);
			return true;
		}
		else
		{
			// If we got here, file is not found on Nexus (either because it was deleted or because we've recovered wrong IDs).
			// But we can still try to restore as much as possible from the file name itself.
			
			if (!nameParsed && !ParseDownloadName(fileItem.GetName(), fileReply))
			{
				return false;
			}
			return true;
		}
	}
	void NexusRepository::OnToolBarMenu(KxMenu& menu)
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KTr("NetworkManager.NXMHandler.Caption"), wxEmptyString, wxITEM_CHECK));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ModNetwork_Nexus));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			ConfigureNXMHandler();
		});
	}
	void NexusRepository::OnDownloadMenu(KxMenu& menu, DownloadItem* download)
	{
		if (download && download->GetModNetwork() == &m_Nexus)
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KTr("NetworkManager.Nexus.CopyNXM")));
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				Utility::CopyTextToClipboard(ConstructNXM(download->GetNetworkModInfo(), download->GetTargetGame()).BuildUnescapedURI());
			});
		}
	}
	
	bool NexusRepository::QueueDownload(const wxString& link)
	{
		GameID gameID;
		NetworkModInfo modInfo;
		NexusNXMLinkData nxmExtraInfo;

		if (ParseNXM(link, gameID, modInfo, nxmExtraInfo))
		{
			if (auto fileInfo = GetModFileInfo(ModRepositoryRequest(modInfo, gameID)))
			{
				ModRepositoryRequest request(modInfo, gameID);
				request.SetExtraInfo(nxmExtraInfo);

				if (auto linkItems = GetFileDownloads(request); !linkItems.empty())
				{
					// Here we should actually select preferred download server based on user choice if we got more than one,
					// but for now just use the first one (it's the preferred server selected in user preferences on Nexus).
					return IDownloadManager::GetInstance()->QueueDownload(*this, linkItems.front(), *fileInfo, gameID);
				}
			}
		}
		return false;
	}
	wxAny NexusRepository::GetDownloadTarget(const wxString& link)
	{
		LoadNXMHandlerOptions();
		
		GameID gameID;
		NetworkModInfo modInfo;
		NexusNXMLinkData nxmExtraInfo;
		if (ParseNXM(link, gameID, modInfo, nxmExtraInfo))
		{
			wxString nexusID = m_Nexus.TranslateGameIDToNetwork(gameID);

			using namespace NXMHandler;
			if (auto value = m_NXMHandlerOptions->GetOption<OptionStore::Instance>(nexusID))
			{
				return IGameInstance::GetShallowInstance(value->ID);
			}
			else if (auto value = m_NXMHandlerOptions->GetOption<OptionStore::Command>(nexusID))
			{
				CmdLine cmdLine;
				cmdLine.Executable = value->Executable;
				cmdLine.Arguments = value->Arguments;
				return cmdLine;
			}
		}
		return {};
	}

	std::optional<ModInfoReply> NexusRepository::GetModInfo(const ModRepositoryRequest& request) const
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/%3"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(request),
												 request.GetModID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply.AsString()).IsSuccessful())
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
		if (!m_Utility.TestRequestError(reply, reply.AsString()).IsSuccessful())
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
		if (!m_Utility.TestRequestError(reply, reply.AsString()).IsSuccessful())
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
		auto reply = GetModFiles2(request, true, false);
		if (reply)
		{
			auto& filesMap = reply->first;

			std::vector<ModFileReply> filesVector;
			filesVector.reserve(filesMap.size());

			for (auto& [id, fileReply]: reply->first)
			{
				filesVector.push_back(std::move(fileReply));
			}
			return filesVector;
		}
		return {};
	}
	auto NexusRepository::GetModFiles2(const ModRepositoryRequest& request, bool files, bool updates) const -> std::optional<GetModFiles2Result>
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/%3/files"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(request),
												 request.GetModID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply.AsString()).IsSuccessful())
		{
			return std::nullopt;
		}

		GetModFiles2Result infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			GetModFiles2Result::first_type filesMap;
			if (files)
			{
				for (const KxJSONObject& value: json["files"])
				{
					ModFileReply& info = filesMap.emplace(value["file_id"].get<ModID::TValue>(), ModFileReply()).first->second;
					info.ModID = request.GetModID();
					m_Utility.ReadFileInfo(value, info);
				}
			}

			GetModFiles2Result::second_type updatesMap;
			if (updates)
			{
				for (const KxJSONObject& value: json["file_updates"])
				{
					ModFileID oldID = value["old_file_id"].get<ModFileID::TValue>();
					NexusModFileUpdateReply& info = updatesMap.emplace(oldID, NexusModFileUpdateReply()).first->second;

					info.OldID = oldID;
					info.NewID = value["new_file_id"].get<ModFileID::TValue>();

					info.OldName = value["old_file_name"].get<wxString>();
					info.NewName = value["new_file_name"].get<wxString>();

					info.UploadedDate = m_Utility.ReadDateTime(value["uploaded_time"]);
				}
			}

			infoVector = {std::move(filesMap), std::move(updatesMap)};
		}
		catch (...)
		{
			return std::nullopt;
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
			query += KxString::Format("?key=%1&expires=%2&user_id=%3", nxmExtraInfo.Key, nxmExtraInfo.Expires, nxmExtraInfo.UserID);
		}
		auto connection = m_Nexus.NewCURLSession(query);

		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply.AsString()).IsSuccessful())
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

				wxString url = value["URI"].get<wxString>();
				m_Utility.ConvertUnicodeEscapes(url);
				info.URI = url;
			}
		}
		catch (...)
		{
			infoVector.clear();
		}
		return infoVector;
	}

	KxURI NexusRepository::ConstructNXM(const NetworkModInfo& modInfo, const GameID& id, const NexusNXMLinkData& linkData) const
	{
		wxString nxm = KxString::Format(wxS("nxm://%1/mods/%2/files/%3"),
										m_Nexus.TranslateGameIDToNetwork(id),
										modInfo.GetModID().GetValue(),
										modInfo.GetFileID().GetValue()
		);
		if (!linkData.IsEmpty())
		{
			nxm += KxString::Format(wxS("?key=%1&expires=%2&user_id=%3"), linkData.Key, linkData.Expires, linkData.UserID);
		}

		nxm.MakeLower();
		return nxm;
	}
	bool NexusRepository::ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData) const
	{
		if (KxURI uri(link); uri.IsOk())
		{
			// Check if it's a well-formed NXM
			if (uri.HasScheme() && uri.HasServer() && uri.HasPath() && uri.GetScheme() == wxS("nxm"))
			{
				gameID = m_Nexus.TranslateGameIDFromNetwork(uri.GetServer());

				// Parse base part of the link
				constexpr auto baseNXM = u8R"(mods\/(\d+)\/files\/(\d+))";
				const wxString& path = uri.GetPath();

				if (wxRegEx regEx(baseNXM, wxRE_ADVANCED|wxRE_ICASE); regEx.Matches(path))
				{
					ModID modID(regEx.GetMatch(path, 1));
					ModFileID fileID(regEx.GetMatch(path, 2));
					modInfo = NetworkModInfo(modID, fileID);
				}

				// If there's a query parse it
				if (const wxString& query = uri.GetQuery(); uri.HasQuery())
				{
					constexpr auto extraInfo = u8R"(key=(.*)&expires=(.*)&user_id=(.*))";
					if (wxRegEx regEx(extraInfo, wxRE_ADVANCED|wxRE_ICASE); regEx.Matches(query))
					{
						linkData.Key = regEx.GetMatch(query, 1);
						linkData.Expires = regEx.GetMatch(query, 2);
						linkData.UserID = regEx.GetMatch(query, 3);
					}
				}

				// Return true if at least game, mod and file IDs are parsed
				return gameID && !modInfo.IsEmpty();
			}
		}
		return false;
	}
	
	void NexusRepository::ConfigureNXMHandler()
	{
		LoadNXMHandlerOptions();
		
		NXMHandler::Dialog dialog(IApplication::GetInstance()->GetActiveWindow(), *m_NXMHandlerOptions);
		dialog.ShowModal();
		m_NXMHandlerOptions->Save(GetNXMHandlerOptions());
	}
}
