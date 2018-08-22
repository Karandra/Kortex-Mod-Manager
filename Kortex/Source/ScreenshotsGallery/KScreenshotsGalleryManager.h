#pragma once
#include "stdafx.h"
#include "KPluggableManager.h"
#include "KScreenshotsGalleryWorkspace.h"
class KWorkspace;
class KMainWindow;
class KScreenshotsGalleryWorkspace;
typedef std::vector<wxBitmapType> KSGMImageTypeArray;

class KScreenshotsGalleryManager: public KPluggableManager
{
	public:
		static const KxStringVector& GetSupportedExtensions();
		static const KSGMImageTypeArray& GetSupportedFormats();
		static bool IsAnimationFile(const wxString& filePath);

	private:
		virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

	public:
		KScreenshotsGalleryManager();
		virtual ~KScreenshotsGalleryManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_PICTURES;
		}

	public:
		virtual KWorkspace* GetWorkspace() const override;
};
