#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>

class KINetFSHandler: public wxInternetFSHandler, public KxSingletonPtr<KINetFSHandler>
{
	private:
		wxString GetCacheFolder() const;
		wxString ExtractFileName(const wxString& location) const;
		wxString ConstructFullPath(const wxString& location) const;
		bool HasCachedCopy(const wxString& location) const;
		wxFSFile* DoOpenFile(const wxString& location) const;

	public:
		KINetFSHandler();
		virtual ~KINetFSHandler();

	public:
		virtual bool CanOpen(const wxString& location) override;
		virtual wxString FindFirst(const wxString& wildcard, int flags) override;
		virtual wxString FindNext() override;
		virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
		wxImage OpenFileAsImage(const wxString& location) const;
};
