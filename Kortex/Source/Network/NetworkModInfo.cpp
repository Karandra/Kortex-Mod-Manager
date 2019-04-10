#include "stdafx.h"
#include "NetworkModInfo.h"

namespace Kortex
{
	wxTextValidator NetworkModInfo::GetValidator()
	{
		wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
		validator.SetCharIncludes(wxS(":0123456789"));
		return validator;
	}

	bool NetworkModInfo::FromString(const wxString& stringValue)
	{
		wxString fileID;
		wxString modID = stringValue.BeforeFirst(wxS(':'), &fileID);

		m_FileID.FromString(fileID);
		return m_ModID.FromString(modID);
	}
}
