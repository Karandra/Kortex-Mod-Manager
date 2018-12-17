#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KxMenu;

namespace Kortex
{
	class IGameInstance;
	class KLocationsManagerConfig
	{
		private:
			KLabeledValue::Vector m_Locations;

		public:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node);

		public:
			KLabeledValue::Vector GetLocations() const;
			bool OpenLocation(const KLabeledValue& entry) const;

			void OnAddMainMenuItems(KxMenu& mainMenu);
	};
}
