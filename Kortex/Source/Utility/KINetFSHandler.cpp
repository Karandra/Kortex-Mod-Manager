#include "stdafx.h"
#include "KINetFSHandler.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include "Utility/Log.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxINet.h>

using namespace Kortex;

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
	Utility::Log::LogMessage("[KINetFSHandler] Attempt to download resource: \"%1\"", location);
	if (HasCachedCopy(location))
	{
		wxString fullPath = ConstructFullPath(location);
		Utility::Log::LogMessage("[KINetFSHandler] Using cached copy: \"%1\"", fullPath);

		KxFileStream* stream = new KxFileStream(fullPath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Everything);
		return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
	}
	else if (KxINet::IsInternetAvailable())
	{
		Utility::Log::LogMessage("[KINetFSHandler] No cached copy, downloading");

		wxString fullPath = ConstructFullPath(location);
		KxFileStream* stream = new KxFileStream(fullPath, KxFileStream::Access::RW, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Everything);

		KxCURLSession session(location);		
		KxCURLStreamReply reply(*stream);
		session.Download(reply);
		if (reply.IsOK() && reply.GetResponseCode() == 200)
		{
			stream->Rewind();

			Utility::Log::LogMessage("[KINetFSHandler] Resource downloaded to: \"%1\"", fullPath);
			return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
		}
	}

	Utility::Log::LogMessage("[KINetFSHandler] Can not download resource: \"%1\"", location);
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
	Utility::Log::LogInfo("[%1] Wildcards: %2, flags: %3", __FUNCTION__, wildcard, flags);
	return wxInternetFSHandler::FindFirst(wildcard, flags);
}
wxString KINetFSHandler::FindNext()
{
	// And this too
	Utility::Log::LogInfo("[%1]", __FUNCTION__);
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
