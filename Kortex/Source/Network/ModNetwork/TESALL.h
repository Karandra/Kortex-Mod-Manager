#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::NetworkManager
{
	class TESALLModNetwork:
		public KxRTTI::IExtendInterface<TESALLModNetwork, IModNetwork>,
		public KxSingletonPtr<TESALLModNetwork>
	{
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
			KxURI GetModPageURI(const ModRepositoryRequest& request) override;
	};
}
