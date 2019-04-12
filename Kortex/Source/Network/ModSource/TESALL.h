#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModSource.h"
#include "TESALLModInfo.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::NetworkManager
{
	class TESALLProvider:
		public KxRTTI::IExtendInterface<TESALLProvider, IModSource>,
		public KxSingletonPtr<TESALLProvider>
	{
		public:
			TESALLProvider();

		public:
			virtual KImageEnum GetIcon() const override;
			virtual wxString GetName() const override;
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURL(const ProviderRequest& request) override;

		public:
			std::unique_ptr<IModInfo> NewModInfo() const override
			{
				return std::make_unique<Nexus::ModInfo>();
			}
			std::unique_ptr<IModFileInfo> NewModFileInfo() const override
			{
				return std::make_unique<Nexus::ModFileInfo>();
			}
			std::unique_ptr<IModDownloadInfo> NewModDownloadInfo() const override
			{
				return std::make_unique<Nexus::ModDownloadInfo>();
			}
			std::unique_ptr<IModEndorsementInfo> NewModEndorsementInfo() const override
			{
				return std::make_unique<Nexus::ModEndorsementInfo>();
			}

			std::unique_ptr<IModInfo> GetModInfo(const ProviderRequest& request) const override;
			std::unique_ptr<IModFileInfo> GetFileInfo(const ProviderRequest& request) const override;
			IModFileInfo::Vector GetFilesList(const ProviderRequest& request) const override;
			IModDownloadInfo::Vector GetFileDownloadLinks(const ProviderRequest& request) const override;
			std::unique_ptr<IModEndorsementInfo> EndorseMod(const ProviderRequest& request, ModEndorsement state) override;
	};
}
