#include "stdafx.h"
#include "Common.h"
#include "IPluginManager.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxComparator.h>

namespace Kortex::PluginManager
{
	StdContentEntry::StdContentEntry(const KxXMLNode& node)
	{
		m_ID = node.GetAttribute(wxS("ID"));
		m_Name = node.GetAttribute(wxS("Name"));
		m_Logo = node.GetAttribute(wxS("Logo"));
	}

	wxString StdContentEntry::GetID() const
	{
		return m_ID;
	}
	wxString StdContentEntry::GetName() const
	{
		return KVarExp(m_Name);
	}
	wxString StdContentEntry::GetLogo() const
	{
		return KVarExp(m_Logo);
	}

	wxString StdContentEntry::GetLogoFullPath() const
	{
		return KVarExp(KxString::Format("%1\\PluginManager\\Logos\\%2\\%3", IApplication::GetInstance()->GetDataFolder(), "$(GameID)", GetLogo()));
	}
}

namespace Kortex::PluginManager
{
	SortingToolEntry::SortingToolEntry(const KxXMLNode& node)
	{
		m_ID = node.GetAttribute("ID");
		m_Name = node.GetAttribute("Name");
		m_Command = node.GetFirstChildElement("Command").GetValue();
	}

	wxString SortingToolEntry::GetID() const
	{
		return m_ID;
	}
	wxString SortingToolEntry::GetName() const
	{
		return KVarExp(m_Name);
	}

	wxString SortingToolEntry::GetExecutable() const
	{
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			//return manager->GetSortingToolsOptions().GetAttribute(m_ID);
		}
		return wxEmptyString;
	}
	void SortingToolEntry::SetExecutable(const wxString& path) const
	{
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			//manager->GetSortingToolsOptions().SetAttribute(m_ID, path);
		}
	}

	wxString SortingToolEntry::GetArguments() const
	{
		return KVarExp(m_Command);
	}
}

namespace Kortex::PluginManager
{
	void Config::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		m_Implementation = node.GetAttribute("Implementation");
		m_PluginImplementation = node.GetAttribute("PluginImplementation");
		m_PluginLimit = node.GetFirstChildElement("Limit").GetAttributeInt("Value", -1);

		// Load std content
		KxXMLNode stdContentNode = node.GetFirstChildElement("StandardContent");
		m_StdandardContent_MainID = stdContentNode.GetAttribute("MainID");
		for (KxXMLNode entryNode = stdContentNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			m_StandardContent.emplace_back(entryNode);
		}

		// Load sorting tools
		for (KxXMLNode entryNode = node.GetFirstChildElement("SortingTools").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			m_SortingTools.emplace_back(entryNode);
		}
	}

	bool Config::HasMainStdContentID() const
	{
		return !m_StdandardContent_MainID.IsEmpty();
	}
	wxString Config::GetMainStdContentID() const
	{
		return KVarExp(m_StdandardContent_MainID);
	}
	const StdContentEntry* Config::GetStandardContent(const wxString& id) const
	{
		auto it = std::find_if(m_StandardContent.begin(), m_StandardContent.end(), [&id](const StdContentEntry& entry)
		{
			return KxComparator::IsEqual(entry.GetID(), id);
		});
		if (it != m_StandardContent.cend())
		{
			return &*it;
		}
		return nullptr;
	}
}
