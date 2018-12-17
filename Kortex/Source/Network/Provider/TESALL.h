#pragma once
#include "stdafx.h"
#include "Network/NetworkConstants.h"
#include "Network/INetworkProvider.h"
#include "TESALLModInfo.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::Network
{
	class TESALLProvider: public INetworkProvider, public KxSingletonPtr<TESALLProvider>
	{
		public:
			static constexpr ProviderID GetTypeID()
			{
				return ProviderIDs::TESALL;
			}

		protected:
			virtual bool DoAuthenticate(wxWindow* window = nullptr) override;
			virtual bool DoValidateAuth(wxWindow* window = nullptr) override;
			virtual bool DoSignOut(wxWindow* window = nullptr) override;
			virtual bool DoIsAuthenticated() const override;

		public:
			TESALLProvider();

		public:
			virtual KImageEnum GetIcon() const override;
			virtual wxString GetName() const override;
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURL(int64_t modID, const wxString& modSignature = wxEmptyString, const GameID& id = GameIDs::NullGameID) override;

			virtual ModInfo GetModInfo(int64_t modID, const GameID& id = GameIDs::NullGameID) const override;
			virtual FileInfo GetFileItem(int64_t modID, int64_t fileID, const GameID& id = GameIDs::NullGameID) const override;
			virtual FileInfo::Vector GetFilesList(int64_t modID, const GameID& id = GameIDs::NullGameID) const override;
			virtual DownloadInfo::Vector GetFileDownloadLinks(int64_t modID, int64_t fileID, const GameID& id = GameIDs::NullGameID) const override;
			virtual EndorsementInfo EndorseMod(int64_t modID, EndorsementState::Value state = EndorsementState::Endorse, const GameID& id = GameIDs::NullGameID) override;
	};
}
