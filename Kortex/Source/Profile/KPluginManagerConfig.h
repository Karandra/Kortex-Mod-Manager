#pragma once
#include "stdafx.h"
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxSingleton.h>
class KProfile;
class KPluginManager;
class KPluginReader;

class KPluginManagerConfigStdContentEntry
{
	public:
		using Vector = std::vector<KPluginManagerConfigStdContentEntry>;

	private:
		wxString m_ID;
		wxString m_Name;
		wxString m_Logo;

	public:
		KPluginManagerConfigStdContentEntry(const KxXMLNode& node);
		~KPluginManagerConfigStdContentEntry();

	public:
		wxString GetID() const;
		wxString GetName() const;
		wxString GetLogo() const;
		wxString GetLogoFullPath() const;
};

//////////////////////////////////////////////////////////////////////////
class KPluginManagerConfigSortingToolEntry
{
	public:
		using Vector = std::vector<KPluginManagerConfigSortingToolEntry>;

	private:
		wxString m_ID;
		wxString m_Name;
		wxString m_Command;

	public:
		KPluginManagerConfigSortingToolEntry(const KxXMLNode& node);
		~KPluginManagerConfigSortingToolEntry();

	public:
		wxString GetID() const;
		wxString GetName() const;
		
		wxString GetExecutable() const;
		void SetExecutable(const wxString& path) const;
		
		wxString GetArguments() const;
};

//////////////////////////////////////////////////////////////////////////
class KLootAPI;
class KPluginManagerConfigLootAPI
{
	private:
		KxLibrary m_Librray;
		wxString m_Branch;
		wxString m_Repository;
		wxString m_FolderName;
		wxString m_LocalGamePath;

		KLootAPI* m_LootInstance = NULL;

	public:
		KPluginManagerConfigLootAPI(KProfile& profile, const KxXMLNode& node);
		~KPluginManagerConfigLootAPI();

	public:
		wxString GetBranch() const;
		wxString GetRepository() const;
		wxString GetFolderName() const;
		wxString GetLocalGamePath() const;
};

//////////////////////////////////////////////////////////////////////////
class KPluginManagerConfig: public KxSingletonPtr<KPluginManagerConfig>
{
	public:
		using StandardContentEntry = KPluginManagerConfigStdContentEntry;
		using SortingToolEntry = KPluginManagerConfigSortingToolEntry;
		using LootAPI = KPluginManagerConfigLootAPI;

	private:
		const wxString m_InterfaceName;
		const wxString m_PluginFileFormat;
		std::unique_ptr<KPluginManager> m_Manager;

		int m_PluginLimit = -1;
		wxString m_StandardContent_MainID;
		StandardContentEntry::Vector m_StandardContent;

		SortingToolEntry::Vector m_SortingTools;
		std::unique_ptr<LootAPI> m_LootAPI;

	public:
		KPluginManagerConfig(KProfile& profile, const KxXMLNode& node);
		~KPluginManagerConfig();

	public:
		bool IsOK() const
		{
			return !m_InterfaceName.IsEmpty();
		}
		const wxString& GetInterfaceName() const
		{
			return m_InterfaceName;
		}
		const wxString& GetPluginFileFormat() const
		{
			return m_PluginFileFormat;
		}

		bool HasPluginLimit() const
		{
			return m_PluginLimit > 0;
		}
		int GetPluginLimit() const
		{
			return m_PluginLimit;
		}

		bool HasMainStdContentID() const;
		wxString GetMainStdContentID() const;

		const StandardContentEntry* GetStandardContent(const wxString& id) const;
		bool IsStandardContent(const wxString& id) const
		{
			return GetStandardContent(id) != NULL;
		}

		bool HasSortingTools() const
		{
			return !m_SortingTools.empty();
		}
		const SortingToolEntry::Vector& GetSortingTools() const
		{
			return m_SortingTools;
		}

		bool HasLootAPI() const
		{
			return m_LootAPI != NULL;
		}
		const LootAPI& GetLootAPI() const
		{
			return *m_LootAPI;
		}
};
