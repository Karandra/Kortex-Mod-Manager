#pragma once
#include "stdafx.h"
#include "BethesdaPluginManager.h"
class KxINI;

namespace Kortex::PluginManager
{
	class BethesdaPluginManagerMW: public RTTI::IMultiInterface<BethesdaPluginManagerMW, BethesdaPluginManager>
	{
		private:
			const wxString m_PluginsListFile;

		private:
			void ReadOrderMW(const KxINI& ini);
			void ReadActiveMW(const KxINI& ini);
			void WriteOrderMW(KxINI& ini) const;
			void WriteActiveMW(KxINI& ini) const;

		protected:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			virtual void LoadNativeActiveBG() override;
			virtual void LoadNativeOrderBG() override;
			virtual void SaveNativeOrderBG() const override;
			wxString GetMorrowindINI() const;	

		public:
			BethesdaPluginManagerMW();
			virtual ~BethesdaPluginManagerMW();

		public:
			virtual wxString GetPluginRootRelativePath(const wxString& fileName) const override
			{
				return m_PluginsLocation + wxS('\\') + fileName;
			}

			virtual void Save() const override;
			virtual void Load() override;
			virtual void LoadNativeOrder() override;
	};
}
