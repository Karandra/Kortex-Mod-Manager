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
		wxString GetMorrowindINI() const;	

		virtual void LoadNativeActiveBG() override;
		virtual void LoadNativeOrderBG() override;
		virtual void SaveNativeOrderBG() const override;

	public:
		KPluginManagerBethesdaMW(const wxString& interfaceName, const KxXMLNode& configNode);
		virtual ~KPluginManagerBethesdaMW();

	public:
		virtual wxString GetPluginRootRelativePath(const wxString& fileName) const override
		{
			return m_PluginsLocation + wxS('\\') + fileName;
		}

		virtual void Save() const override;
		virtual void Load() override;
		virtual void LoadNativeOrder() override;
};
