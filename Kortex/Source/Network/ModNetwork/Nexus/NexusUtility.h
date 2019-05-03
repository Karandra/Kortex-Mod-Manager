#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "NexusModInfo.h"
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxHTTPStatusCode.h>
class KxCURLReplyBase;

namespace Kortex::NetworkManager
{
	class NexusModNetwork;

	class NexusUtility: public KxComponentOf<NexusModNetwork>
	{
		public:
			using TJsonValue = typename nlohmann::json::value_type;

		private:
			NexusModNetwork& m_Nexus;

		public:
			NexusUtility(NexusModNetwork& nexus)
				:m_Nexus(nexus)
			{
			}

		public:
			void ConvertChangeLog(wxString& changeLog) const;
			void ConvertDisplayName(wxString& name) const;
			void ConvertUnicodeEscapes(wxString& source) const;

			wxDateTime ReadDateTime(const TJsonValue& json) const;
			void ReadFileInfo(const TJsonValue& json, ModFileReply& info) const;
			void ReadGameInfo(const TJsonValue& json, NexusGameReply& info) const;

			void ReportRequestError(const wxString& message) const;
			void ReportRequestQuoteReached() const;
			KxHTTPStatusValue TestRequestError(const KxCURLReplyBase& reply, const wxString& message = {}, bool noErrorReport = false) const;
	};
}
