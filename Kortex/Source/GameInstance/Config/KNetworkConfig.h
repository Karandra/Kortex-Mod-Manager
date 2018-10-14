#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;

class KNetworkConfig: public KxSingletonPtr<KNetworkConfig>
{
	private:
		wxString m_NexusID;
		int64_t m_SteamID = -1;

	public:
		KNetworkConfig(KGameInstance& profile, const KxXMLNode& node);
		~KNetworkConfig();

	public:
		bool HasNexusID() const
		{
			return !m_NexusID.IsEmpty();
		}
		const wxString& GetNexusID() const
		{
			return m_NexusID;
		}

		bool HasSteamID() const
		{
			return m_SteamID > 0;
		}
		int64_t GetSteamID() const
		{
			return m_SteamID;
		}
};
