#pragma once
#include "stdafx.h"
#include "Application/IManager.h"

namespace Kortex::PluginManager
{
	class StdContentItem final
	{
		public:
			using Vector = std::vector<StdContentItem>;

		private:
			wxString m_ID;
			wxString m_Name;
			wxString m_Logo;

		public:
			StdContentItem(const KxXMLNode& node);

		public:
			wxString GetID() const;
			wxString GetName() const;
			wxString GetLogo() const;
			wxString GetLogoFullPath() const;
	};
	class SortingToolItem final
	{
		public:
			using Vector = std::vector<SortingToolItem>;

		private:
			wxString m_ID;
			wxString m_Name;
			wxString m_Command;
			mutable wxString m_Executable;

		public:
			SortingToolItem(const KxXMLNode& node);

		public:
			wxString GetID() const;
			wxString GetName() const;
			
			wxString GetExecutable() const;
			void SetExecutable(const wxString& path) const;
			
			wxString GetArguments() const;
	};
	class Config final
	{
		private:
			wxString m_Implementation;
			wxString m_PluginImplementation;

			int m_PluginLimit = -1;
			wxString m_StdandardContent_MainID;
			StdContentItem::Vector m_StandardContent;
			SortingToolItem::Vector m_SortingTools;

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

			const StdContentItem* GetStandardContent(const wxString& id) const;
			bool IsStandardContent(const wxString& id) const
			{
				return GetStandardContent(id) != nullptr;
			}

			bool HasSortingTools() const
			{
				return !m_SortingTools.empty();
			}
			const SortingToolItem::Vector& GetSortingTools() const
			{
				return m_SortingTools;
			}
	};
}
