#include "stdafx.h"
#include "DefaultProgramManager.h"
#include "DefaultProgramEntry.h"
#include <Kortex/Application.hpp>
#include <Kortex/Common/Programs.hpp>

namespace Kortex::ProgramManager
{
	namespace ProgramManager::Internal
	{
		const SimpleManagerInfo TypeInfo("DefaultProgramManager", "ProgramManager.Name");
	}

	void DefaultProgramManager::OnInit()
	{
		LoadUserPrograms();
	}
	void DefaultProgramManager::OnExit()
	{
		SaveUserPrograms();
	}
	void DefaultProgramManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadProgramsFromXML(m_DefaultPrograms, managerNode.GetFirstChildElement("DefaultPrograms"));
	}

	void DefaultProgramManager::LoadUserPrograms()
	{
		KxXMLNode node = GetAInstanceOption(Application::OName::UserPrograms).GetConfigNode();
		LoadProgramsFromXML(m_UserPrograms, node);
	}
	void DefaultProgramManager::SaveUserPrograms() const
	{
		auto option = GetAInstanceOption(Application::OName::UserPrograms);
		KxXMLNode rootNode = option.GetConfigNode();
		rootNode.ClearNode();

		for (const auto& entry: m_UserPrograms)
		{
			KxXMLNode node = rootNode.NewElement(wxS("Entry"));
			entry->Save(node);
		}
		option.NotifyChange();
	}

	void DefaultProgramManager::LoadDefaultPrograms()
	{
		for (const auto& programEntry: m_DefaultPrograms)
		{
			m_UserPrograms.emplace_back(std::make_unique<DefaultProgramEntry>(static_cast<DefaultProgramEntry&>(*programEntry)));
		}
	}

	std::unique_ptr<IProgramEntry> DefaultProgramManager::NewProgram()
	{
		return std::make_unique<DefaultProgramEntry>();
	}
	void DefaultProgramManager::RemoveProgram(IProgramEntry& programEntry)
	{
		auto it = std::remove_if(m_UserPrograms.begin(), m_UserPrograms.end(), [&programEntry](const auto& item)
		{
			return item.get() == &programEntry;
		});
		m_UserPrograms.erase(it, m_UserPrograms.end());
	}
}
