#pragma once
#include "stdafx.h"
#include "BethesdaPluginManager.h"
#include "BethesdaPlugin2.h"

namespace Kortex::PluginManager
{
	class BethesdaPluginManager2: public RTTI::IMultiInterface<BethesdaPluginManager2, BethesdaPluginManager>
	{
		protected:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			virtual bool CheckExtension(const wxString& name) const override;
			virtual void LoadNativeActiveBG() override;

			virtual wxString OnWriteToLoadOrder(const IGamePlugin& plugin) const override;
			virtual wxString OnWriteToActiveOrder(const IGamePlugin& plugin) const override;

			intptr_t CountLightActiveBefore(const IGamePlugin& plugin) const;
			virtual wxString OnFormatPriority(const IGamePlugin& modEntry, intptr_t value) const override;
			virtual intptr_t OnGetPluginDisplayPriority(const IGamePlugin& plugin) const;

		public:
			BethesdaPluginManager2();
			virtual ~BethesdaPluginManager2();

		public:
			virtual std::unique_ptr<IGamePlugin> CreatePlugin(const wxString& fullPath, bool isActive) const override;
	};
}
