#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KProfile;
class KScreenshotsGalleryManager;

class KScreenshotsGalleryConfig: public KxSingletonPtr<KScreenshotsGalleryConfig>
{
	private:
		KScreenshotsGalleryManager* m_Manager = NULL;
		bool m_RequiresVFS = true;
		KxStringVector m_Locations;

	public:
		KScreenshotsGalleryConfig(KProfile& profile, KxXMLNode& node);
		~KScreenshotsGalleryConfig();

	public:
		bool IsOK() const
		{
			return m_Manager != NULL;
		}
		KScreenshotsGalleryManager* GetManager() const
		{
			return m_Manager;
		}
		bool IsRequiresVFS() const
		{
			return m_RequiresVFS;
		}
		const KxStringVector& GetLocations() const
		{
			return m_Locations;
		}
};
