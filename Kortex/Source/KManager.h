#pragma once
#include "stdafx.h"
#include "KApp.h"
#include "KEvents.h"
class KWorkspace;
class KMainWindow;
class KPluggableManager;

class KManager: public wxEvtHandler
{
	friend class KMainWindow;

	private:
		typedef std::vector<KManager*> InstancesListType;
		static InstancesListType ms_Instances;

	public:
		static InstancesListType& GetInstances();

	public:
		KManager();
		virtual ~KManager();

	public:
		virtual KWorkspace* GetWorkspace() const
		{
			return NULL;
		}
		bool HasWorkspace() const
		{
			return GetWorkspace() != NULL;
		}

		virtual void Load()
		{
		}
		virtual void Save() const
		{
		}

		virtual wxString GetID() const = 0;
		virtual wxString GetName() const = 0;
		virtual wxString GetVersion() const = 0;
		virtual KImageEnum GetImageID() const = 0;

		virtual const KPluggableManager* ToPluggableManager() const
		{
			return NULL;
		}
		virtual KPluggableManager* ToPluggableManager()
		{
			return NULL;
		}
};
