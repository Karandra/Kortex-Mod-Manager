#pragma once
#include "Framework.hpp"
#include "Application/AppOption.h"

namespace kxf
{
	class XMLDocument;
}
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
	class KORTEX_API BasicOption: public AppOption
	{
		protected:
			void Create(Disposition disposition, kxf::XMLDocument& xml, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IApplication& app, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IGameInstance& instance, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IGameProfile& profile, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IWorkspace& workspace, const kxf::String& branch = {});
			void Create(Disposition disposition, kxf::XMLDocument& xml, const IMainWindow& mainWindow, const kxf::String& branch = {});
	};
}

namespace Kortex::Application
{
	class KORTEX_API GlobalOption: public BasicOption
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

	class KORTEX_API InstanceOption: public BasicOption
	{
		private:
			kxf::XMLDocument& GetXML(IGameInstance& instance) const;

		protected:
			template<class... Args>
			void Initialize(IGameInstance* instance, Args&&... arg)
			{
				if (instance)
				{
					Create(Disposition::Instance, GetXML(*instance), std::forward<Args>(arg)...);
					AssignInstance(*instance);
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

	class KORTEX_API ActiveInstanceOption: public InstanceOption
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

	class KORTEX_API ProfileOption: public BasicOption
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

	class KORTEX_API ActiveProfileOption: public ProfileOption
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
