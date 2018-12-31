#include "stdafx.h"
#include "IScreenshotsGallery.h"
#include <Kortex/ScreenshotsGallery.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

namespace Kortex
{
	namespace ScreenshotsGallery::Internal
	{
		const SimpleManagerInfo TypeInfo("ScreenshotsGallery", "ScreenshotsGallery.Name");
	}

	const KxStringVector& IScreenshotsGallery::GetSupportedExtensions()
	{
		static const KxStringVector ms_SupportedExtensions = {"*.jpg", "*.jpeg", "*.bmp", "*.png", "*.gif", "*.ico", "*.tga", "*.tif"};
		return ms_SupportedExtensions;
	}
	const ScreenshotsGallery::SupportedTypesVector& IScreenshotsGallery::GetSupportedFormats()
	{
		static const ScreenshotsGallery::SupportedTypesVector ms_SupportedFormats = {wxBITMAP_TYPE_JPEG, wxBITMAP_TYPE_JPEG, wxBITMAP_TYPE_BMP, wxBITMAP_TYPE_PNG, wxBITMAP_TYPE_ICO, wxBITMAP_TYPE_TGA, wxBITMAP_TYPE_TIF};
		return ms_SupportedFormats;
	}
	bool IScreenshotsGallery::IsAnimationFile(const wxString& filePath)
	{
		return KAux::IsSingleFileExtensionMatches(filePath, wxS("gif")) || KAux::IsSingleFileExtensionMatches(filePath, wxS("ani"));
	}

	IScreenshotsGallery::IScreenshotsGallery()
		:ManagerWithTypeInfo(GameDataModule::GetInstance())
	{
	}
}
