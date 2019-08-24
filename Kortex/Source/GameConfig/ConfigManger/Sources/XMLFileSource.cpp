#include "stdafx.h"
#include "XMLFileSource.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameConfig
{
	bool XMLFileSource::Open()
	{
		if (!XMLSource::IsOpened())
		{
			KxFileStream stream(GetResolvedFilePath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
			if (stream.IsOk() && GetXML().Load(stream))
			{
				XMLSource::Open();
				return true;
			}
		}
		return false;
	}
	bool XMLFileSource::Save()
	{
		KxFileStream stream(GetResolvedFilePath(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		if (stream.IsOk())
		{
			return GetXML().Save(stream);
		}
	}
	void XMLFileSource::Close()
	{
		XMLSource::Close();
	}
}
