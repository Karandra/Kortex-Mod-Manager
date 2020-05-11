#include "stdafx.h"
#include "Common.h"
#include "GameData/IPluginManager.h"
#include "Application/AppOption.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxComparator.h>

namespace
{
	KortexDefOption(SortingTools);
}

namespace Kortex::PluginManager
{
	StdContentItem::StdContentItem(const KxXMLNode& node)
	{
		m_ID = node.GetAttribute(wxS("ID"));
		m_Name = node.GetAttribute(wxS("Name"));
		m_Logo = node.GetAttribute(wxS("Logo"));
	}

	wxString StdContentItem::GetID() const
	{
		return m_ID;
	}
	wxString StdContentItem::GetName() const
	{
		return KVarExp(m_Name);
	}
	wxString StdContentItem::GetLogo() const
	{
		return KVarExp(m_Logo);
	}

	wxString StdContentItem::GetLogoFullPath() const
	{
		return KVarExp(KxString::Format("%1\\PluginManager\\Logos\\%2\\%3", IApplication::GetInstance()->GetDataFolder(), "$(GameID)", GetLogo()));
	}
}

namespace Kortex::PluginManager
{
	SortingToolItem::SortingToolItem(const KxXMLNode& node)
	{
		m_ID = node.GetAttribute("ID");
		m_Name = node.GetAttribute("Name");
		m_Command = node.GetFirstChildElement("Command").GetValue();
	}

	wxString SortingToolItem::GetID() const
	{
		return m_ID;
	}
	wxString SortingToolItem::GetName() const
	{
		return KVarExp(m_Name);
	}

	wxString SortingToolItem::GetExecutable() const
	{
		if (m_Executable.IsEmpty())
		{
			if (IPluginManager* manager = IPluginManager::GetInstance())
			{
				KxXMLNode option = manager->GetAInstanceOption(SortingTools).GetNode();
				for (KxXMLNode node = option.GetFirstChildElement(); node; node = node.GetNextSiblingElement())
				{
					if (node.GetAttribute("ID") == m_ID)
					{
						m_Executable = node.GetFirstChildElement("Executable").GetValue();
						break;
					}
				}
			}
		}
		return m_Executable;
	}
	void SortingToolItem::SetExecutable(const wxString& path) const
	{
		if (m_Executable != path)
		{
			m_Executable = path;

			if (IPluginManager* manager = IPluginManager::GetInstance())
			{
				KxXMLNode option = manager->GetAInstanceOption(SortingTools).GetNode();
				for (KxXMLNode node = option.GetFirstChildElement(); node; node = node.GetFirstChildElement())
				{
					if (node.GetAttribute("ID") == m_ID)
					{
						node.GetFirstChildElement("Executable").SetValue(path);
						return;
					}
				}

				if (!path.IsEmpty())
				{
					KxXMLNode node = option.NewElement("Item");
					node.SetAttribute("ID", m_ID);
					node.NewElement("Executable").SetValue(path);
				}
			}
		}
	}

	wxString SortingToolItem::GetArguments() const
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
	const StdContentItem* Config::GetStandardContent(const wxString& id) const
	{
		auto it = std::find_if(m_StandardContent.begin(), m_StandardContent.end(), [&id](const StdContentItem& entry)
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
