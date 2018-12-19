#pragma once
#include "stdafx.h"
#include "Application/IAppOption.h"
class KWorkspace;
class KMainWindow;
class KInstallWizardDialog;

namespace Kortex
{
	class IApplication;
	class IModule;
	class IManager;
	class IGameInstance;
	class IGameProfile;
}

namespace Kortex::Application
{
	class BasicOption: public IAppOption
	{
		protected:
			KxXMLNode m_Node;

		protected:
			void Create(Disposition disposition, KxXMLDocument& xml, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const IApplication& app, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const IGameInstance& instance, const wxString& branch = wxEmptyString);
			void Create(Disposition disposition, KxXMLDocument& xml, const IGameProfile& profile, const wxString& branch = wxEmptyString);
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
			}
	};

	class InstanceOption: public BasicOption
	{
		private:
			IConfigurableGameInstance* GetConfigurableInstance(IGameInstance* instance) const;
			KxXMLDocument& GetXML(IConfigurableGameInstance* instance) const;

		public:
			template<class... Args> InstanceOption(IGameInstance* instance, Args&&... arg)
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
			IGameInstance* GetActiveInstance() const;

		public:
			template<class... Args> ActiveInstanceOption(Args&&... arg)
				:InstanceOption(GetActiveInstance(), std::forward<Args>(arg)...)
			{
			}
	};

	class ProfileOption: public BasicOption
	{
		private:
			KxXMLDocument& GetXML(IGameProfile& profile) const;

		public:
			template<class... Args> ProfileOption(IGameProfile* profile, Args&&... arg)
			{
				Create(Disposition::Profile, GetXML(profile), std::forward<Args>(arg)...);
				AssignProfile(&profile);
			}
	};

	class ActiveProfileOption: public ProfileOption
	{
		private:
			IGameProfile* GetActiveProfile() const;

		public:
			template<class... Args> ActiveProfileOption(Args&&... arg)
				:ProfileOption(GetActiveProfile(), std::forward<Args>(arg)...)
			{
			}
	};
}

namespace Kortex::Application
{
	template<class T> class WithOptions
	{
		protected:
			T& NCSelf() const
			{
				return *const_cast<T*>(static_cast<const T*>(this));
			}

		public:
			GlobalOption GetGlobalOption(const wxString& branch = wxEmptyString) const
			{
				return GlobalOption(NCSelf(), branch);
			}
			ActiveInstanceOption GetActiveInstanceOption(const wxString& branch = wxEmptyString) const
			{
				return ActiveInstanceOption(NCSelf(), branch);
			}
			ActiveProfileOption GetActiveProfileOption(const wxString& branch = wxEmptyString) const
			{
				return ActiveProfileOption(NCSelf(), branch);
			}

			
			#if 0
			ProfileOption GetProfileOption(const wxString& branch = wxEmptyString) const
			{
				return ProfileOption(GetNCSelf(), &GetNCSelf(), branch);
			}
			#endif
	};

	template<class T> class WithInstanceOptions: public WithOptions<T>
	{
		public:
			InstanceOption GetInstanceOption(const wxString& branch = wxEmptyString) const
			{
				return InstanceOption(&NCSelf(), NCSelf(), branch);
			}
	};

	template<class T> class WithProfileOptions: public WithOptions<T>
	{
		public:
			ProfileOption GetProfileOption(const wxString& branch = wxEmptyString) const
			{
				return ProfileOption(&NCSelf(), NCSelf(), branch);
			}
	};
}
