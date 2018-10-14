#include "stdafx.h"
#include "KScreenshotsGalleryConfig.h"
#include "ScreenshotsGallery/KScreenshotsGalleryManager.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"

KScreenshotsGalleryConfig::KScreenshotsGalleryConfig(KGameInstance& profile, const KxXMLNode& node)
{
	for (KxXMLNode entryNode = node.GetFirstChildElement("Locations").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_Locations.emplace_back(V(entryNode.GetValue()));
	}
	m_Manager = new KScreenshotsGalleryManager();
}
KScreenshotsGalleryConfig::~KScreenshotsGalleryConfig()
{
	delete m_Manager;
}
