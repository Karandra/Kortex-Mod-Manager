#pragma once
#include "stdafx.h"
#include "KPluggableManager.h"
#include "Profile/KSaveManagerConfig.h"
#include "KEvents.h"
#include <KxFramework/KxSingleton.h>
class KWorkspace;
class KxXMLNode;

class KSaveManager: public KPluggableManager, public KxSingletonPtr<KSaveManager>
{
	friend class KSaveManagerConfig;

	private:
		KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

		void OnSavesLocationChanged(KModListEvent& event);

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

		wxString GetSavesLocation() const;
};
