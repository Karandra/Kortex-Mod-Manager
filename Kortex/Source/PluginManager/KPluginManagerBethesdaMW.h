#pragma once
#include "stdafx.h"
#include "KPluginManagerBethesda.h"
class KxINI;

class KPluginManagerBethesdaMW: public KPluginManagerBethesda
{
	private:
		const wxString m_PluginsListFile;

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
		KPluginManagerBethesdaMW(const wxString& interfaceName, const KxXMLNode& configNode);
		virtual ~KPluginManagerBethesdaMW();

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
			return m_PluginsLocation + '\\' + fileName;
		}

		virtual void Save() const override;
		virtual void Load() override;
		virtual void LoadNativeOrder() override;
};
