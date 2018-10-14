#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;
class KScreenshotsGalleryManager;

class KScreenshotsGalleryConfig: public KxSingletonPtr<KScreenshotsGalleryConfig>
{
	private:
		KScreenshotsGalleryManager* m_Manager = NULL;
		KxStringVector m_Locations;

	public:
		KScreenshotsGalleryConfig(KGameInstance& profile, const KxXMLNode& node);
		~KScreenshotsGalleryConfig();

	public:
		bool IsOK() const
		{
			return m_Manager != NULL;
		}

		const KxStringVector& GetLocations() const
		{
			return m_Locations;
		}
};
