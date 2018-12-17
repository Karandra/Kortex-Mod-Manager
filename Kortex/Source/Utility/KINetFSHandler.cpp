#include "stdafx.h"
#include "KINetFSHandler.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxINet.h>

wxString KINetFSHandler::GetCacheFolder() const
{
	return Kortex::INetworkManager::GetInstance()->GetCacheFolder();
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
	wxLogMessage("[KINetFSHandler] Attempt to download resource: \"%s\"", location);
	if (HasCachedCopy(location))
	{
		wxString fullPath = ConstructFullPath(location);
		wxLogMessage("[KINetFSHandler] Using cached copy: \"%s\"", fullPath);

		KxFileStream* stream = new KxFileStream(fullPath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Everything);
		return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
	}
	else if (KxINet::IsInternetAvailable())
	{
		wxLogMessage("[KINetFSHandler] No cached copy, downloading");

		wxString fullPath = ConstructFullPath(location);
		KxFileStream* stream = new KxFileStream(fullPath, KxFileStream::Access::RW, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Everything);

		KxCURLSession session(location);		
		KxCURLStreamReply reply(*stream);
		session.Download(reply);
		if (reply.IsOK() && reply.GetResponseCode() == 200)
		{
			stream->Rewind();

			wxLogMessage("[KINetFSHandler] Resource downloaded to: \"%s\"", fullPath);
			return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
		}
	}

	wxLogError("[KINetFSHandler] Can not download resource: \"%s\"", location);
	return nullptr;
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
	wxLogMessage("[%s] Wildcards: %s, flags: %d", __FUNCTION__, wildcard, flags);
	return wxInternetFSHandler::FindFirst(wildcard, flags);
}
wxString KINetFSHandler::FindNext()
{
	// And this too
	wxLogMessage("[%s]", __FUNCTION__);
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
