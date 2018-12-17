#pragma once
#include "stdafx.h"
#include "Application/IManager.h"

namespace Kortex::PluginManager
{
	class StdContentEntry
	{
		public:
			using Vector = std::vector<StdContentEntry>;

		private:
			wxString m_ID;
			wxString m_Name;
			wxString m_Logo;

		public:
			StdContentEntry(const KxXMLNode& node);

		public:
			wxString GetID() const;
			wxString GetName() const;
			wxString GetLogo() const;
			wxString GetLogoFullPath() const;
	};
	class SortingToolEntry
	{
		public:
			using Vector = std::vector<SortingToolEntry>;

		private:
			wxString m_ID;
			wxString m_Name;
			wxString m_Command;

		public:
			SortingToolEntry(const KxXMLNode& node);

		public:
			wxString GetID() const;
			wxString GetName() const;
		
			wxString GetExecutable() const;
			void SetExecutable(const wxString& path) const;
		
			wxString GetArguments() const;
	};
	class Config
	{
		private:
			wxString m_Implementation;
			wxString m_PluginImplementation;

			int m_PluginLimit = -1;
			wxString m_StdandardContent_MainID;
			StdContentEntry::Vector m_StandardContent;
			SortingToolEntry::Vector m_SortingTools;

		public:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node);

		public:
			const wxString& GetManagerImplementation() const
			{
				return m_Implementation;
			}
			const wxString& GetPluginImplementation() const
			{
				return m_PluginImplementation;
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

			const StdContentEntry* GetStandardContent(const wxString& id) const;
			bool IsStandardContent(const wxString& id) const
			{
				return GetStandardContent(id) != nullptr;
			}

			bool HasSortingTools() const
			{
				return !m_SortingTools.empty();
			}
			const SortingToolEntry::Vector& GetSortingTools() const
			{
				return m_SortingTools;
			}
	};
}
