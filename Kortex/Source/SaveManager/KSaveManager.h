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
		const KSaveManagerConfig* m_ProfileSaveManager = NULL;
		KWorkspace* m_Workspace = NULL;
		bool m_RequiresVFS = true;

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
		const KSaveManagerConfig* GetProfileSaveManager() const
		{
			return m_ProfileSaveManager;
		}

		virtual KWorkspace* GetWorkspace() const override
		{
			return m_Workspace;
		}
		virtual bool IsActiveVFSNeeded() const override
		{
			return m_RequiresVFS;
		}
		virtual bool Save();
		virtual bool Load();
		virtual bool Reload();
};
