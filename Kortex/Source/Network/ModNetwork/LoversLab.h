#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::NetworkManager
{
	class LoversLabModNetwork:
		public KxRTTI::ExtendInterface<LoversLabModNetwork, IModNetwork>,
		public KxSingletonPtr<LoversLabModNetwork>
	{
		KxDecalreIID(LoversLabModNetwork, {0xfd953e5d, 0x2c04, 0x4e82, {0x9f, 0x59, 0x6c, 0xc6, 0x91, 0xb6, 0xa3, 0x73}});

		private:
			wxString GetAPIURL() const;

		protected:
			void OnInit() override
			{
			}
			void OnExit() override
			{
			}
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode) override
			{
			}

		public:
			LoversLabModNetwork();

		public:
			ResourceID GetIcon() const override;
			wxString GetName() const override;

			wxString TranslateGameIDToNetwork(const GameID& id = {}) const override
			{
				return {};
			}
			GameID TranslateGameIDFromNetwork(const wxString& id) const override
			{
				return {};
			}

			KxURI GetModPageBaseURI(const GameID& id = {}) const override;
			KxURI GetModPageURI(const ModRepositoryRequest& request) const override;
	};
}
