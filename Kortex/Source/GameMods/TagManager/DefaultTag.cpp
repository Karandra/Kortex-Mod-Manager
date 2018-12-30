#include "stdafx.h"
#include "DefaultTag.h"
#include "DefaultTagManager.h"

namespace Kortex::ModTagManager
{
	std::optional<wxString> DefaultTag::TryGetTranslatedName(const wxString& id)
	{
		return Translation::TryGetString(wxS("TagManager.Tag.") + id);
	}

	wxString DefaultTag::GetName() const
	{
		if (m_Name.IsEmpty())
		{
			auto name = TryGetTranslatedName(m_ID);
			return name ? *name : m_ID;
		}
		return m_Name;
	}
	void DefaultTag::SetName(const wxString& name)
	{
		m_Name = name;
	}
}
