#pragma once
#include "stdafx.h"
#include "BethesdaPluginManager.h"
#include "BethesdaPlugin2.h"

namespace Kortex::PluginManager
{
	class Bethesda2DisplayModel;

	class BethesdaPluginManager2: public RTTI::IExtendInterface<BethesdaPluginManager2, BethesdaPluginManager>
	{
		friend class Bethesda2DisplayModel;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			bool CheckExtension(const wxString& name) const override;
			void LoadNativeOrderBG() override;
			void LoadNativeActiveBG() override;
			void SaveNativeOrderBG() const override;

			wxString OnWriteToLoadOrder(const IGamePlugin& plugin) const override;
			wxString OnWriteToActiveOrder(const IGamePlugin& plugin) const override;

			intptr_t CountLightActiveBefore(const IGamePlugin& plugin) const;
			intptr_t OnGetPluginDisplayPriority(const IGamePlugin& plugin) const override;

		public:
			BethesdaPluginManager2();
			virtual ~BethesdaPluginManager2();

		public:
			std::unique_ptr<IGamePlugin> CreatePlugin(const wxString& fullPath, bool isActive) override;
			std::unique_ptr<PluginManager::IDisplayModel> CreateDisplayModel() override;
	};
}
