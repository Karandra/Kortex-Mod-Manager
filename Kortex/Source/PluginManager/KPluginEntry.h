#pragma once
#include "stdafx.h"
#include "KPluginEntryTypesInterface.h"
#include "KPluginReader.h"
#include <KxFramework/KxRTTI.h>
class KModEntry;
class KPluginManagerConfigStdContentEntry;

class KPluginEntry: public KxRTTI::DynamicCastAsIs<KPluginEntry>
{
	public:
		using Vector = std::vector<std::unique_ptr<KPluginEntry>>;
		using RefVector = std::vector<KPluginEntry*>;

	private:
		mutable const KPluginManagerConfigStdContentEntry* m_StdContent = NULL;
		mutable std::unique_ptr<KPluginReader> m_Reader;

	protected:
		virtual void OnPluginRead(const KPluginReader& reader) = 0;

	public:
		virtual ~KPluginEntry();

	public:
		virtual bool IsOK() const = 0;

	public:
		virtual wxString GetName() const = 0;
		virtual void SetName(const wxString& name) = 0;
		
		virtual wxString GetFullPath() const = 0;
		virtual void SetFullPath(const wxString& fullPath) = 0;

		virtual bool CanToggleEnabled() const = 0;
		virtual bool IsEnabled() const = 0;
		virtual void SetEnabled(bool isActive) = 0;

		virtual const KModEntry* GetParentMod() const = 0;
		virtual void SetParentMod(const KModEntry* modEntry) = 0;

		bool IsStdContent() const
		{
			return GetStdContentEntry() != NULL;
		}
		const KPluginManagerConfigStdContentEntry* GetStdContentEntry() const;
		
		bool HasReader() const
		{
			return GetReader() != NULL;
		}
		const KPluginReader* GetReader() const
		{
			return m_Reader.get();
		}
		bool ReadPluginData();
};
