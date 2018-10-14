#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;

class KLocationsManagerConfig: public KxSingletonPtr<KLocationsManagerConfig>
{
	private:
		KLabeledValueArray m_Locations;

	public:
		KLocationsManagerConfig(KGameInstance& profile, const KxXMLNode& node);
		~KLocationsManagerConfig();

	public:
		KLabeledValueArray GetLocations() const;
		bool OpenLocation(const KLabeledValue& entry) const;
};
