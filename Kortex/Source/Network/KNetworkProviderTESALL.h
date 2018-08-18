#pragma once
#include "stdafx.h"
#include "KNetworkConstants.h"
#include "KNetworkProvider.h"
#include <KxFramework/KxSingleton.h>
class KNetworkProviderTESALL;
class KxCURLSession;

namespace KNetworkProviderTESALLNS
{
	using InfoBase = KNetworkProviderNS::InfoBase;
}

//////////////////////////////////////////////////////////////////////////
class KNetworkProviderTESALL: public KNetworkProvider, public KxSingletonPtr<KNetworkProviderTESALL>
{
	public:
		static constexpr KNetworkProviderID GetTypeID()
		{
			return KNETWORK_PROVIDER_ID_TESALL;
		}

	protected:
		virtual bool DoAuthenticate(wxWindow* window = NULL) override;
		virtual bool DoValidateAuth(wxWindow* window = NULL) override;
		virtual bool DoSignOut(wxWindow* window = NULL) override;
		virtual bool DoIsAuthenticated() const override;

	public:
		KNetworkProviderTESALL(KNetworkProviderID providerID);

	public:
		virtual KNetworkProviderID GetID() const override
		{
			return GetTypeID();
		}
		virtual KImageEnum GetIcon() const override;
		virtual wxString GetName() const override;
		virtual wxString GetGameID(const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual wxString GetModURLBasePart(const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual wxString GetModURL(int64_t modID, const wxString& modSignature = wxEmptyString, const KProfileID& id = KProfileIDs::NullProfileID) override;

		virtual ModInfo GetModInfo(int64_t modID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual FileInfo GetFileInfo(int64_t modID, int64_t fileID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual FileInfo::Vector GetFilesList(int64_t modID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual DownloadInfo::Vector GetFileDownloadLinks(int64_t modID, int64_t fileID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual EndorsedInfo EndorseMod(int64_t modID, EndorsementState::Value state = EndorsementState::Endorse, const KProfileID& id = KProfileIDs::NullProfileID) override;
};
