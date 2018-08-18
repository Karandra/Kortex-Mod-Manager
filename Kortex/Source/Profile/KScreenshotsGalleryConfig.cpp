#include "stdafx.h"
#include "KScreenshotsGalleryConfig.h"
#include "ScreenshotsGallery/KScreenshotsGalleryManager.h"
#include "KProfile.h"
#include "KApp.h"

KxSingletonPtr_Define(KScreenshotsGalleryConfig);

KScreenshotsGalleryConfig::KScreenshotsGalleryConfig(KProfile& profile, KxXMLNode& node)
{
	m_RequiresVFS = node.GetAttributeBool("RequiresVFS");
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
