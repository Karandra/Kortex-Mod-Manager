#include "stdafx.h"
#include "NetworkWxFSHandler.h"
#include "INetworkManager.h"
#include "Utility/Log.h"
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxINet.h>

namespace Kortex::NetworkManager
{
	wxString NetworkWxFSHandler::GetCacheFolder() const
	{
		return m_NetworkManager.GetCacheFolder();
	}
	wxString NetworkWxFSHandler::ExtractFileName(const wxString& location) const
	{
		return KxINet::SplitURL(location).FileName;
	}
	wxString NetworkWxFSHandler::ConstructFullPath(const wxString& location) const
	{
		return GetCacheFolder() + wxS('\\') + ExtractFileName(location);
	}
	wxFSFile* NetworkWxFSHandler::DoOpenFile(const wxString& location) const
	{
		Utility::Log::LogMessage("[NetworkWxFSHandler] Attempt to download resource: \"%1\"", location);
		
		KxFile file = GetCachedCopyFile(location);
		if (file.IsFileExist() && IsNeverThan(file.GetFileTime(KxFILETIME_MODIFICATION), wxTimeSpan::Days(7)))
		{
			wxString fullPath = ConstructFullPath(location);
			Utility::Log::LogMessage("[NetworkWxFSHandler] Using cached copy: \"%1\"", fullPath);

			KxFileStream* stream = new KxFileStream(fullPath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
			return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
		}
		else if (KxINet::IsInternetAvailable())
		{
			Utility::Log::LogMessage("[NetworkWxFSHandler] No cached copy, downloading");

			wxString fullPath = ConstructFullPath(location);
			KxFileStream* stream = new KxFileStream(fullPath, KxFileStream::Access::RW, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
			KxCURLStreamReply reply(*stream);

			auto session = m_NetworkManager.NewCURLSession(location);
			session->Download(reply);
			if (reply.IsOK())
			{
				stream->Rewind();

				Utility::Log::LogMessage("[NetworkWxFSHandler] Resource downloaded to: \"%1\"", fullPath);
				return new wxFSFile(stream, fullPath, wxEmptyString, GetAnchor(location), wxFileName(fullPath).GetModificationTime());
			}
		}

		// File is invalid, delete it.
		file.RemoveFile();

		Utility::Log::LogMessage("[NetworkWxFSHandler] Can not download resource: \"%1\"", location);
		return nullptr;
	}
	
	KxFile NetworkWxFSHandler::GetCachedCopyFile(const wxString& location) const
	{
		return ConstructFullPath(location);
	}
	bool NetworkWxFSHandler::IsNeverThan(const wxDateTime& fileDate, const wxTimeSpan& span) const
	{
		wxDateTime currentDate = wxDateTime::Now();
		if (fileDate > currentDate)
		{
			// File from the future? Return false to delete this file and download a new one.
			return false;
		}
		else
		{
			return currentDate - fileDate <= span;
		}
	}

	NetworkWxFSHandler::NetworkWxFSHandler(INetworkManager& networkManager)
		:m_NetworkManager(networkManager)
	{
		KxFile(GetCacheFolder()).CreateFolder();
	}
	NetworkWxFSHandler::~NetworkWxFSHandler()
	{
	}

	bool NetworkWxFSHandler::CanOpen(const wxString& location)
	{
		// Base class doesn't seems to support HTTPS protocol
		return GetProtocol(location) == wxS("https") || wxInternetFSHandler::CanOpen(location);
	}
	wxString NetworkWxFSHandler::FindFirst(const wxString& wildcard, int flags)
	{
		// This don't seems to be called at all
		Utility::Log::LogInfo("[%1] Wildcards: %2, flags: %3", __FUNCTION__, wildcard, flags);
		return wxInternetFSHandler::FindFirst(wildcard, flags);
	}
	wxString NetworkWxFSHandler::FindNext()
	{
		// And this too
		Utility::Log::LogInfo("[%1]", __FUNCTION__);
		return wxInternetFSHandler::FindNext();
	}
	wxFSFile* NetworkWxFSHandler::OpenFile(wxFileSystem& fs, const wxString& location)
	{
		return DoOpenFile(location);
	}
}
