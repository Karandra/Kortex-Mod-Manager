#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KProfile;

class KLocationsManagerConfig: public KxSingletonPtr<KLocationsManagerConfig>
{
	private:
		KLabeledValueArray m_Locations;

	public:
		KLocationsManagerConfig(KProfile& profile, KxXMLNode& node);
		~KLocationsManagerConfig();

	public:
		const KLabeledValueArray& GetLocations() const
		{
			return m_Locations;
		}
		bool OpenLocation(const KLabeledValue& entry) const;
};
