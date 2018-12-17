#pragma once
#include "stdafx.h"
#include "GameData/IScreenshotsGallery.h"

namespace Kortex::ScreenshotsGallery
{
	class Config
	{
		private:
			KxStringVector m_Locations;

		public:
			void OnLoadInstance(IGameInstance& profile, const KxXMLNode& node);

		public:
			KxStringVector GetLocations() const;
	};
}

namespace Kortex::ScreenshotsGallery
{
	class DefaultScreenshotsGallery: public IScreenshotsGallery
	{
		private:
			Config m_Config;

		private:
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;

		public:
			const Config& GetConfig() const override
			{
				return m_Config;
			}
			KWorkspace* GetWorkspace() const override;
	};
}
