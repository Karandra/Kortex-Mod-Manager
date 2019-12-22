#include "stdafx.h"
#include "DefaultScreenshotsGallery.h"
#include <Kortex/ScreenshotsGallery.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/Common.h"

namespace Kortex::ScreenshotsGallery
{
	void DefaultScreenshotsGallery::CreateWorkspaces()
	{
		new Workspace();
	}

	void DefaultScreenshotsGallery::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}
	void DefaultScreenshotsGallery::OnInit()
	{
	}
	void DefaultScreenshotsGallery::OnExit()
	{
	}

	IWorkspace::RefVector DefaultScreenshotsGallery::EnumWorkspaces() const
	{
		return ToWorkspacesList(Workspace::GetInstance());
	}
}

namespace Kortex::ScreenshotsGallery
{
	void Config::OnLoadInstance(IGameInstance& profile, const KxXMLNode& node)
	{
		for (KxXMLNode entryNode = node.GetFirstChildElement("Locations").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			m_Locations.emplace_back(entryNode.GetValue());
		}
	}
	KxStringVector Config::GetLocations() const
	{
		return Utility::ExpandVariablesInVector(m_Locations);
	}
}
