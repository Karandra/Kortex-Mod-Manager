#pragma once
#include "Framework.hpp"
#include "Application/AppOption.h"

namespace Kortex
{
	class IApplication;
	class IWorkspace;
	class IMainWindow;
	class IModule;
	class IManager;
	class IGameInstance;
	class IGameProfile;
}
namespace Kortex::InstallWizard
{
	class WizardDialog;
}

namespace Kortex::Application
{
	class BasicOption: public AppOption
	{
		protected:
			void Create(Disposition disposition, kxf::XMLDocument& xml, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IApplication& app, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IGameInstance& instance, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IGameProfile& profile, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IManager& manager, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IWorkspace& workspace, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IMainWindow& mainWindow, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const InstallWizard::WizardDialog& installWizard, const kxf::String& branch = {});
	};
}

namespace Kortex::Application
{
	class GlobalOption: public BasicOption
	{
		private:
			kxf::XMLDocument& GetXML() const;

		public:
			template<class... Args>
			GlobalOption(Args&&... arg)
			{
				Create(Disposition::Global, GetXML(), std::forward<Args>(arg)...);
			}
	};

	class InstanceOption: public BasicOption
	{
		private:
			IConfigurableGameInstance* GetConfigurableInstance(IGameInstance& instance) const;
			kxf::XMLDocument& GetXML(IConfigurableGameInstance& instance) const;

		protected:
			template<class... Args>
			void Initialize(IGameInstance* instance, Args&&... arg)
			{
				if (instance)
				{
					if (IConfigurableGameInstance* configurableInstance = GetConfigurableInstance(*instance))
					{
						Create(Disposition::Instance, GetXML(*configurableInstance), std::forward<Args>(arg)...);
						AssignInstance(*configurableInstance);
					}
				}
			}

		protected:
			InstanceOption() = default;

		public:
			template<class... Args>
			InstanceOption(IGameInstance& instance, Args&&... arg)
			{
				Initialize(&instance, std::forward<Args>(arg)...);
			}
	};

	class ActiveInstanceOption: public InstanceOption
	{
		private:
			IGameInstance* GetActiveInstance() const;

		public:
			template<class... Args>
			ActiveInstanceOption(Args&&... arg)
			{
				Initialize(GetActiveInstance(), std::forward<Args>(arg)...);
			}
	};

	class ProfileOption: public BasicOption
	{
		private:
			kxf::XMLDocument* GetXML(IGameProfile& profile) const;

		protected:
			template<class... Args>
			void Initialize(IGameProfile* profile, Args&&... arg)
			{
				if (profile)
				{
					if (auto xml = GetXML(*profile))
					{
						Create(Disposition::Profile, *xml, std::forward<Args>(arg)...);
					}
					AssignProfile(*profile);
				}
			}

		protected:
			ProfileOption() = default;

		public:
			template<class... Args>
			ProfileOption(IGameProfile& profile, Args&&... arg)
			{
				Initialize(&profile, std::forward<Args>(arg)...);
			}
	};

	class ActiveProfileOption: public ProfileOption
	{
		private:
			IGameProfile* GetActiveProfile() const;

		public:
			template<class... Args>
			ActiveProfileOption(Args&&... arg)
			{
				Initialize(GetActiveProfile(), std::forward<Args>(arg)...);
			}
	};
}

namespace Kortex::Application
{
	template<class T>
	class WithOptions
	{
		protected:
			const T& CSelf() const
			{
				return *static_cast<const T*>(this);
			}
			T& NCSelf() const
			{
				return const_cast<T&>(CSelf());
			}

		public:
			template<class... Args>
			GlobalOption GetGlobalOption(Args&&... arg) const
			{
				return GlobalOption(NCSelf(), AppOption::MakeXPath(std::forward<Args>(arg)...));
			}
			
			template<class... Args>
			ActiveInstanceOption GetActiveInstanceOption(Args&&... arg) const
			{
				return ActiveInstanceOption(NCSelf(), AppOption::MakeXPath(std::forward<Args>(arg)...));
			}
			
			template<class... Args>
			ActiveProfileOption GetActiveProfileOption(Args&&... arg) const
			{
				return ActiveProfileOption(NCSelf(), AppOption::MakeXPath(std::forward<Args>(arg)...));
			}
	};

	template<class T>
	class WithInstanceOptions: public WithOptions<T>
	{
		public:
			template<class... Args>
			InstanceOption GetInstanceOption(Args&&... arg) const
			{
				return InstanceOption(this->NCSelf(), this->NCSelf(), AppOption::MakeXPath(std::forward<Args>(arg)...));
			}
	};

	template<class T>
	class WithProfileOptions: public WithOptions<T>
	{
		public:
			template<class... Args>
			ProfileOption GetProfileOption(Args&&... arg) const
			{
				return ProfileOption(this->NCSelf(), this->NCSelf(), AppOption::MakeXPath(std::forward<Args>(arg)...));
			}
	};
}
