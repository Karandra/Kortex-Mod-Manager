#pragma once
#include "stdafx.h"
#include "Application/IPluggableManager.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	namespace ScreenshotsGallery
	{
		using SupportedTypesVector = std::vector<wxBitmapType>;
		class Config;
	}
	namespace ScreenshotsGallery::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IScreenshotsGallery:
		public ManagerWithTypeInfo<IPluggableManager, ScreenshotsGallery::Internal::TypeInfo>,
		public KxSingletonPtr<IScreenshotsGallery>
	{
		public:
			static const KxStringVector& GetSupportedExtensions();
			static const ScreenshotsGallery::SupportedTypesVector& GetSupportedFormats();
			static bool IsAnimationFile(const wxString& filePath);

		public:
			IScreenshotsGallery();

		public:
			virtual const ScreenshotsGallery::Config& GetConfig() const = 0;
	};
}
