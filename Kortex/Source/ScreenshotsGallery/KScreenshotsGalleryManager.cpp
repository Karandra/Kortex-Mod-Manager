#include "stdafx.h"
#include "KScreenshotsGalleryManager.h"
#include "KScreenshotsGalleryWorkspace.h"
#include "GameInstance/KGameInstance.h"
#include "GameInstance/Config/KScreenshotsGalleryConfig.h"
#include "KApp.h"

const KxStringVector& KScreenshotsGalleryManager::GetSupportedExtensions()
{
	static const KxStringVector ms_SupportedExtensions = {"*.jpg", "*.jpeg", "*.bmp", "*.png", "*.gif", "*.ico", "*.tga", "*.tif"};
	return ms_SupportedExtensions;
}
const KSGMImageTypeArray& KScreenshotsGalleryManager::GetSupportedFormats()
{
	static const KSGMImageTypeArray ms_SupportedFormats = {wxBITMAP_TYPE_JPEG, wxBITMAP_TYPE_JPEG, wxBITMAP_TYPE_BMP, wxBITMAP_TYPE_PNG, wxBITMAP_TYPE_ICO, wxBITMAP_TYPE_TGA, wxBITMAP_TYPE_TIF};
	return ms_SupportedFormats;
}
bool KScreenshotsGalleryManager::IsAnimationFile(const wxString& filePath)
{
	return KAux::IsSingleFileExtensionMatches(filePath, "gif") || KAux::IsSingleFileExtensionMatches(filePath, "ani");
}

KWorkspace* KScreenshotsGalleryManager::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KScreenshotsGalleryWorkspace(mainWindow, this);
}

KScreenshotsGalleryManager::KScreenshotsGalleryManager()
{
}
KScreenshotsGalleryManager::~KScreenshotsGalleryManager()
{
}

wxString KScreenshotsGalleryManager::GetID() const
{
	return "KScreenshotsGalleryManager";
}
wxString KScreenshotsGalleryManager::GetName() const
{
	return T("ScreenshotsGallery.Name");
}
wxString KScreenshotsGalleryManager::GetVersion() const
{
	return "1.0";
}

KWorkspace* KScreenshotsGalleryManager::GetWorkspace() const
{
	return KScreenshotsGalleryWorkspace::GetInstance();
}
