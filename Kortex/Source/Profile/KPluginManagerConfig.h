#pragma once
#include "stdafx.h"
#include "PluginManager/KPMPluginReader.h"
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxSingleton.h>
class KProfile;
class KPluginManager;
class KPMPluginReader;

class KPluginManagerConfigStandardContentEntry
{
	public:
		using Vector = std::vector<KPluginManagerConfigStandardContentEntry>;

	private:
		wxString m_ID;
		wxString m_Name;
		wxString m_Logo;

	public:
		KPluginManagerConfigStandardContentEntry(KxXMLNode& node);
		~KPluginManagerConfigStandardContentEntry();

	public:
		const wxString& GetID() const
		{
			return m_ID;
		}
		const wxString& GetName() const
		{
			return m_Name;
		}
		const wxString& GetLogo() const
		{
			return m_Logo;
		}
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
		KPluginManagerConfigSortingToolEntry(KxXMLNode& node);
		~KPluginManagerConfigSortingToolEntry();

	public:
		const wxString& GetID() const
		{
			return m_ID;
		}
		const wxString& GetName() const
		{
			return m_Name;
		}
		
		wxString GetExecutable() const;
		void SetExecutable(const wxString& path) const;
		
		const wxString& GetArguments() const
		{
			return m_Command;
		}
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
		const wxString& GetBranch() const
		{
			return m_Branch;
		}
		const wxString& GetRepository() const
		{
			return m_Repository;
		}
		const wxString& GetFolderName() const
		{
			return m_FolderName;
		}
		const wxString& GetLocalGamePath() const
		{
			return m_LocalGamePath;
		}
};

//////////////////////////////////////////////////////////////////////////
class KPluginManagerConfig: public KxSingletonPtr<KPluginManagerConfig>
{
	public:
		using StandardContentEntry = KPluginManagerConfigStandardContentEntry;
		using SortingToolEntry = KPluginManagerConfigSortingToolEntry;
		using LootAPI = KPluginManagerConfigLootAPI;

	private:
		const wxString m_InterfaceName;
		const wxString m_PluginFileFormat;
		KPluginManager* m_Manager = NULL;

		int m_PluginLimit = -1;
		wxString m_StandardContent_MainID;
		StandardContentEntry::Vector m_StandardContent;

		SortingToolEntry::Vector m_SortingTools;
		std::unique_ptr<LootAPI> m_LootAPI;

	public:
		KPluginManagerConfig(KProfile& profile, KxXMLNode& node);
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
		KPluginManager* GetManager() const;
		
		const wxString& GetPluginFileFormat() const
		{
			return m_PluginFileFormat;
		}
		KPMPluginReader* GetPluginReader() const;

		bool HasPluginLimit() const
		{
			return m_PluginLimit > 0;
		}
		int GetPluginLimit() const
		{
			return m_PluginLimit;
		}

		bool HasMainStdContentID() const
		{
			return !m_StandardContent_MainID.IsEmpty();
		}
		const wxString& GetMainStdContentID() const
		{
			return m_StandardContent_MainID;
		}

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
			return *m_LootAPI.get();
		}
};
