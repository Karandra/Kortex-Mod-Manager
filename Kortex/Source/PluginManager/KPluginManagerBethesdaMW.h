#pragma once
#include "stdafx.h"
#include "KPluginManagerBethesdaGeneric.h"
class KxINI;

class KPluginManagerBethesdaMorrowind: public KPluginManagerBethesdaGeneric
{
	private:
		wxString m_PluginsListFile;

	private:
		void ReadOrderMW(const KxINI& ini);
		void ReadActiveMW(const KxINI& ini);
		void WriteOrderMW(KxINI& ini) const;
		void WriteActiveMW(KxINI& ini) const;

	protected:
		virtual void LoadNativeActiveBG() override;
		virtual void LoadNativeOrderBG() override;
		virtual void SaveNativeOrderBG() const override;

	public:
		KPluginManagerBethesdaMorrowind(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig);
		virtual ~KPluginManagerBethesdaMorrowind();

	public:
		virtual bool ShouldChangeFileModificationDate() const override
		{
			return true;
		}
		virtual bool ShouldSortByFileModificationDate() const override
		{
			return true;
		}

		virtual wxString GetPluginRootRelativePath(const wxString& fileName) const override
		{
			return "Data Files\\" + fileName;
		}

		virtual bool Save() override;
		virtual bool Load() override;
		virtual bool LoadNativeOrder() override;
};
