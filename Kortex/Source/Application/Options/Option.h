#pragma once
#include "stdafx.h"
#include "Application/IAppOption.h"
class KWorkspace;
class KMainWindow;
class KInstallWizardDialog;

namespace Kortex
{
	class IGameInstance;
	class IApplication;
	class IModule;
	class IManager;
}

namespace Kortex::Application
{
	class BasicOption: public IAppOption
	{
		public:
			enum class Disposition
			{
				Global,
				Instance,
			};

		protected:
			KxXMLNode m_Node;

		protected:
			void Create(Disposition disposition, KxXMLDocument& xml, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const IApplication& app, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const IModule& module, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const IManager& manager, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const KWorkspace& workspace, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const KMainWindow& mainWindow, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const KInstallWizardDialog& installWizard, const wxString& branch = wxEmptyString);
	};
}

namespace Kortex::Application
{
	class GlobalOption: public BasicOption
	{
		private:
			KxXMLDocument& GetXML() const;

		public:
			template<class... Args> GlobalOption(Args&&... arg)
			{
				Create(Disposition::Global, GetXML(), std::forward<Args>(arg)...);
				AssignActiveInstance();
			}
	};

	class InstanceOption: public BasicOption
	{
		private:
			IConfigurableGameInstance* GetConfigurableInstance(const IGameInstance& instance) const;
			KxXMLDocument& GetXML(IConfigurableGameInstance* instance) const;

		public:
			template<class... Args> InstanceOption(const IGameInstance& instance, Args&&... arg)
			{
				if (IConfigurableGameInstance* configurableInstance = GetConfigurableInstance(instance))
				{
					Create(Disposition::Instance, GetXML(configurableInstance), std::forward<Args>(arg)...);
					AssignInstance(configurableInstance);
				}
			}
	};

	class ActiveInstanceOption: public InstanceOption
	{
		private:
			IGameInstance& GetActiveInstance() const;

		public:
			template<class... Args> ActiveInstanceOption(Args&&... arg)
				:InstanceOption(GetActiveInstance(), std::forward<Args>(arg)...)
			{
			}
	};
}

namespace Kortex::Application
{
	template<class T> class WithOptions
	{
		public:
			GlobalOption GetGlobalOption(const wxString& branch = wxEmptyString) const
			{
				return GlobalOption(*static_cast<const T*>(this), branch);
			}
			ActiveInstanceOption GetInstanceOption(const wxString& branch = wxEmptyString) const
			{
				return ActiveInstanceOption(*static_cast<const T*>(this), branch);
			}
	};
}
