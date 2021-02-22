#pragma once
#include "Framework.hpp"
#include "Application/AppOption.h"

namespace kxf
{
	class XMLDocument;
}
namespace Kortex
{
	class IModule;
	class IGameInstance;
	class IGameProfile;
}

namespace Kortex::Application
{
	class KORTEX_API BasicOption: public AppOption
	{
		private:
			void CreateFromAny(Disposition disposition, kxf::XMLDocument& xml, const kxf::IObject& ref, const kxf::String& branch, bool readOnly);

		protected:
			void Create(Disposition disposition, kxf::XMLDocument& xml, const kxf::String& branch = {});
			void Create(Disposition disposition, const kxf::XMLDocument& xml, const kxf::String& branch = {});

			void Create(Disposition disposition, kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch = {});
			void Create(Disposition disposition, const kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch = {});

			void Create(Disposition disposition, kxf::XMLDocument& xml, const kxf::IObject& ref, const kxf::String& branch = {})
			{
				CreateFromAny(disposition, xml, ref, branch, false);
			}
			void Create(Disposition disposition, const kxf::XMLDocument& xml, const kxf::IObject& ref, const kxf::String& branch = {})
			{
				CreateFromAny(disposition, const_cast<kxf::XMLDocument&>(xml), ref, branch, true);
			}
	};
}

namespace Kortex::Application
{
	namespace Private
	{
		class KORTEX_API GlobalOption: public BasicOption
		{
			protected:
				kxf::XMLDocument& GetXML();
		};
	}

	class GlobalOptionRO final: public Private::GlobalOption
	{
		protected:
			bool DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA = AsCDATA::Auto) override
			{
				return false;
			}
			bool DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty) override
			{
				return false;
			}
			AppOption ConstructElement(const kxf::String& XPath) override
			{
				return {};
			}

		public:
			GlobalOptionRO(const kxf::IObject& ref, const kxf::String& branch = {})
			{
				Create(Disposition::Global, const_cast<const kxf::XMLDocument&>(GetXML()), ref, branch);
			}
	};
	class GlobalOptionRW final: public Private::GlobalOption
	{
		public:
			GlobalOptionRW(const kxf::IObject& ref, const kxf::String& branch = {})
			{
				Create(Disposition::Global, GetXML(), ref, branch);
			}
	};
}
namespace Kortex::Application
{
	namespace Private
	{
		class KORTEX_API InstanceOption: public BasicOption
		{
			protected:
				kxf::XMLDocument& GetXML(IGameInstance& instance);
				const kxf::XMLDocument& GetXML(const IGameInstance& instance) const;

				template<class TSelf, class TGameInstance>
				static void Initialize(TSelf& self, TGameInstance* instance, const kxf::IObject& ref, const kxf::String& branch = {})
				{
					if (instance)
					{
						self.Create(Disposition::Instance, self.GetXML(*instance), ref, branch);
						self.AssignInstance(const_cast<IGameInstance&>(*instance));
					}
				}
		};
	}

	class InstanceOptionRO final: public Private::InstanceOption
	{
		protected:
			bool DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA = AsCDATA::Auto) override
			{
				return false;
			}
			bool DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty) override
			{
				return false;
			}
			AppOption ConstructElement(const kxf::String& XPath) override
			{
				return {};
			}

		public:
			InstanceOptionRO(const IGameInstance& instance, const kxf::IObject& ref, const kxf::String& branch = {})
			{
				Initialize(*this, &instance, ref, branch);
			}
	};
	class InstanceOptionRW final: public Private::InstanceOption
	{
		public:
			InstanceOptionRW(IGameInstance& instance, const kxf::IObject& ref, const kxf::String& branch = {})
			{
				Initialize(*this, &instance, ref, branch);
			}
	};
}
namespace Kortex::Application
{
	namespace Private
	{
		class KORTEX_API ProfileOption: public BasicOption
		{
			protected:
				kxf::XMLDocument& GetXML(IGameProfile& profile);
				const kxf::XMLDocument& GetXML(const IGameProfile& profile) const;

				template<class TSelf, class TGameProfile>
				static void Initialize(TSelf& self, TGameProfile* profile, const kxf::IObject& ref, const kxf::String& branch = {})
				{
					if (profile)
					{
						self.Create(Disposition::Profile, self.GetXML(*profile), ref, branch);
						self.AssignProfile(const_cast<IGameProfile&>(*profile));
					}
				}
		};
	}

	class ProfileOptionRO final: public Private::ProfileOption
	{
		protected:
			bool DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA = AsCDATA::Auto) override
			{
				return false;
			}
			bool DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty) override
			{
				return false;
			}
			AppOption ConstructElement(const kxf::String& XPath) override
			{
				return {};
			}

		public:
			ProfileOptionRO(const IGameProfile& profile, const kxf::IObject& ref, const kxf::String& branch = {})
			{
				Initialize(*this, &profile, ref, branch);
			}
	};
	class ProfileOptionRW final: public Private::ProfileOption
	{
		public:
			ProfileOptionRW(const IGameProfile& profile, const kxf::IObject& ref, const kxf::String& branch = {})
			{
				Initialize(*this, &profile, ref, branch);
			}
	};
}

namespace Kortex::Application
{
	template<class T>
	class WithOptions
	{
		private:
			const T& CSelf() const
			{
				return static_cast<const T&>(*this);
			}
			T& NCSelf()
			{
				return static_cast<T&>(*this);
			}

		public:
			const GlobalOptionRO ReadGlobalOption(const kxf::String& branch = {}) const
			{
				return {CSelf(), branch};
			}
			GlobalOptionRW WriteGlobalOption(const kxf::String& branch = {})
			{
				return {CSelf(), branch};
			}
	};

	template<class T>
	class WithInstanceOptions: public WithOptions<T>
	{
		private:
			const T& CSelf() const
			{
				return static_cast<const T&>(*this);
			}
			T& NCSelf()
			{
				return static_cast<T&>(*this);
			}

		public:
			const InstanceOptionRO ReadInstanceOption(const kxf::String& branch = {}) const
			{
				return {CSelf(), CSelf(), branch};
			}
			InstanceOptionRW WriteInstanceOption(const kxf::String& branch = {})
			{
				return {NCSelf(), NCSelf(), branch};
			}
	};

	template<class T>
	class WithProfileOptions: public WithOptions<T>
	{
		private:
			const T& CSelf() const
			{
				return static_cast<const T&>(*this);
			}
			T& NCSelf()
			{
				return static_cast<T&>(*this);
			}

		public:
			const ProfileOptionRO ReadProfileOption(const kxf::String& branch = {}) const
			{
				return {CSelf(), CSelf(), branch};
			}
			ProfileOptionRW WriteProfileOption(const kxf::String& branch = {})
			{
				return {NCSelf(), NCSelf(), branch};
			}
	};
}
