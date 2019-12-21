#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::NetworkManager
{
	class TESALLModNetwork:
		public KxRTTI::ExtendInterface<TESALLModNetwork, IModNetwork>,
		public KxSingletonPtr<TESALLModNetwork>
	{
		KxDecalreIID(TESALLModNetwork, {0xc4255aa9, 0x9087, 0x49e5, {0xb3, 0xf5, 0x36, 0x67, 0x21, 0x60, 0x6f, 0xbc}});

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
			TESALLModNetwork();

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
