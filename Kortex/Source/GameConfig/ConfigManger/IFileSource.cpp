#include "stdafx.h"
#include "IFileSource.h"
#include <Kortex/ModManager.hpp>

namespace Kortex::GameConfig
{
	wxString IFileSource::ResolveFSLocation(const wxString& path) const
	{
		return IModDispatcher::GetInstance()->ResolveLocationPath(path);
	}

	wxString IFileSource::GetResolvedFileName() const
	{
		return ResolveFSLocation(KVarExp(GetFileName()));
	}
	wxString IFileSource::GetResolvedFilePath() const
	{
		return ResolveFSLocation(KVarExp(GetFilePath()));
	}
}
