#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModSource.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::NetworkManager
{
	class LoversLabSource:
		public KxRTTI::IExtendInterface<LoversLabSource, IModSource>,
		public KxSingletonPtr<LoversLabSource>
	{
		private:
			wxString GetAPIURL() const;

		public:
			LoversLabSource();

		public:
			virtual KImageEnum GetIcon() const override;
			virtual wxString GetName() const override;
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURL(const ModRepositoryRequest& request) override;
	};
}
