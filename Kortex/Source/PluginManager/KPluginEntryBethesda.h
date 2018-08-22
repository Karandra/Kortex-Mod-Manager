#pragma once
#include "stdafx.h"
#include "KPluginEntry.h"
class KPluginReaderBethesda;

class KPluginEntryBethesda: public KPluginEntry, public KPluginEntryTypesInterface::Bethesda
{
	public:
		using Vector = std::vector<std::unique_ptr<KPluginEntryBethesda>>;
		using RefVector = std::vector<KPluginEntryBethesda*>;

	protected:
		virtual void OnPluginRead(const KPluginReader& reader) override;

		void SetLight(bool value)
		{
			m_IsLight = value;
		}
		void SetMaster(bool value)
		{
			m_IsMaster = value;
		}

	private:
		wxString m_Name;
		wxString m_FullPath;
		bool m_IsEnabled = false;
		const KModEntry* m_ParentMod = NULL;

		bool m_IsMaster = false;
		bool m_IsLight = false;

	public:
		KPluginEntryBethesda(const wxString& name, bool isActive)
			:m_Name(name), m_IsEnabled(isActive)
		{
		}

	public:
		virtual bool IsOK() const override
		{
			return !m_Name.IsEmpty() && !m_FullPath.IsEmpty();
		}

		virtual wxString GetName() const override
		{
			return m_Name;
		}
		virtual void SetName(const wxString& name) override
		{
			m_Name = name;
		}
		
		virtual wxString GetFullPath() const override
		{
			return m_FullPath;
		}
		virtual void SetFullPath(const wxString& fullPath)
		{
			m_FullPath = fullPath;
		}

		virtual bool CanToggleEnabled() const override;
		virtual bool IsEnabled() const override
		{
			return m_IsEnabled;
		}
		virtual void SetEnabled(bool isActive) override
		{
			m_IsEnabled = isActive;
		}

		virtual const KModEntry* GetParentMod() const override
		{
			return m_ParentMod;
		}
		virtual void SetParentMod(const KModEntry* modEntry) override
		{
			m_ParentMod = modEntry;
		}

	public:
		virtual bool IsMaster() const override
		{
			return m_IsMaster;
		}
		virtual bool IsLight() const override
		{
			return m_IsLight;
		}
};
