#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KProfile;
class KPackageManager;

class KPackageManagerConfig: public KxSingletonPtr<KPackageManagerConfig>
{
	private:
		KPackageManager* m_Manager = NULL;

	public:
		KPackageManagerConfig(KProfile& profile, KxXMLNode& node);
		virtual ~KPackageManagerConfig();

	public:
		KPackageManager* GetManager() const
		{
			return m_Manager;
		}
};
