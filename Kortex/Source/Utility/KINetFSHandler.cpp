#include "stdafx.h"
#include "KINetFSHandler.h"
#include "Network/KNetwork.h"
#include "KApp.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxINet.h>

wxString KINetFSHandler::GetCacheFolder() const
{
	return KNetwork::GetInstance()->GetCacheFolder();
}
wxString KINetFSHandler::ExtractFileName(const wxString& location) const
{
	return KxINet::SplitURL(location).FileName;
}
wxString KINetFSHandler::ConstructFullPath(const wxString& location) const
{
	return GetCacheFolder() + '\\' + ExtractFileName(location);
}
bool KINetFSHandler::HasCachedCopy(const wxString& location) const
{
	return wxFileName::FileExists(ConstructFullPath(location));
}
wxFSFile* KINetFSHandler::DoOpenFile(const wxString& location) const
{
	KLogMessage("[KINetFSHandler] Attempt to download resource: \"%s\"", location);
	if (HasCachedCopy(location))
	{
		wxString fullPath = ConstructFullPath(location);
		KLogMessage("[KINetFSHandler] Using cached copy: \"%s\"", fullPath);

		KxFileStream* stream = new KxFileStream(fullPath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_ALL);
		return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
	}
	else if (KxINet::IsInternetAvailable())
	{
		KLogMessage("[KINetFSHandler] No cached copy, downloading");

		wxString fullPath = ConstructFullPath(location);
		KxFileStream* stream = new KxFileStream(fullPath, KxFS_ACCESS_RW, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_ALL);

		KxCURLSession session(location);		
		KxCURLStreamReply reply(*stream);
		session.Download(reply);
		if (reply.IsOK() && reply.GetResponseCode() == 200)
		{
			// Rewind stream
			stream->Seek(0, KxFS_SEEK_BEGIN);

			KLogMessage("[KINetFSHandler] Resource downloaded to: \"%s\"", fullPath);
			return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
		}
	}

	wxLogError("[KINetFSHandler] Can not download resource: \"%s\"", location);
	return NULL;
}

KINetFSHandler::KINetFSHandler()
{
	KxFile(GetCacheFolder()).CreateFolder();
}
KINetFSHandler::~KINetFSHandler()
{
}

bool KINetFSHandler::CanOpen(const wxString& location)
{
	return GetProtocol(location) == "https" || wxInternetFSHandler::CanOpen(location);
}
wxString KINetFSHandler::FindFirst(const wxString& wildcard, int flags)
{
	// This don't seems to be called at all
	KLogMessage("[%s] Wildcards: %s, flags: %d", __FUNCTION__, wildcard, flags);
	return wxInternetFSHandler::FindFirst(wildcard, flags);
}
wxString KINetFSHandler::FindNext()
{
	// And this too
	KLogMessage("[%s]", __FUNCTION__);
	return wxInternetFSHandler::FindNext();
}
wxFSFile* KINetFSHandler::OpenFile(wxFileSystem& fs, const wxString& location)
{
	return DoOpenFile(location);
}
wxImage KINetFSHandler::OpenFileAsImage(const wxString& location) const
{
	std::unique_ptr<wxFSFile> file(DoOpenFile(location));
	if (file)
	{
		return wxImage(*file->GetStream(), GetMimeTypeFromExt(location));
	}
	return wxNullImage;
}
