#include "stdafx.h"
#include "NetworkModInfo.h"

namespace Kortex
{
	bool NetworkModInfo::FromString(const wxString& stringValue)
	{
		wxString fileID;
		wxString modID = stringValue.BeforeFirst(wxS(':'), &fileID);

		m_FileID.FromString(fileID);
		return m_ModID.FromString(modID);
	}
}
