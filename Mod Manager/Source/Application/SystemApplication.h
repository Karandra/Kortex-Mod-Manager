#include "Framework.hpp"
#include <kxf/Application/GUIApplication.h>
#include <kxf/Localization/LocalizationPackageStack.h>

namespace Kortex
{
	class SystemApplication: public kxf::RTTI::ImplementInterface<SystemApplication, kxf::GUIApplication>
	{
		public:
			static SystemApplication& GetInstance() noexcept
			{
				return static_cast<SystemApplication&>(*kxf::ICoreApplication::GetInstance());
			}

		private:
			kxf::LocalizationPackageStack m_LocalizationPackages;

		public:
			bool OnCreate() override;
			bool OnInit() override;

			bool OnMainLoopException() override;
			void OnUnhandledException() override;
			void OnAssertFailure(kxf::String file, int line, kxf::String function, kxf::String condition, kxf::String message) override;

			const kxf::ILocalizationPackage& GetLocalizationPackage() const override
			{
				return m_LocalizationPackages;
			}
	};
}
