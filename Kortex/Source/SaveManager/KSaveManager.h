#pragma once
#include "stdafx.h"
#include "KPluggableManager.h"
#include "Profile/KSaveManagerConfig.h"
#include <KxFramework/KxSingleton.h>
class KxXMLNode;
class KWorkspace;

class KSaveManager: public KPluggableManager, public KxSingletonPtr<KSaveManager>
{
	friend class KSaveManagerConfig;

	private:
		KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

	public:
		KSaveManager(const KxXMLNode& configNode, const KSaveManagerConfig* profileSaveManager);
		virtual ~KSaveManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_JAR;
		}

	public:
		virtual KWorkspace* GetWorkspace() const override;
		
		virtual bool Save();
		virtual bool Load();
		virtual bool Reload();
};
