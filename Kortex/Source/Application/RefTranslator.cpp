#include "stdafx.h"
#include "RefTranslator.h"
#include <KxFramework/KxTranslation.h>

namespace Kortex
{
	ITranslator::OpString Kortex::RefTranslator::DoGetString(const wxString& id) const
	{
		bool isSuccess = false;
		wxString value = m_TranslationRef.GetString(id, &isSuccess);
		if (isSuccess)
		{
			return value;
		}
		return std::nullopt;
	}
	ITranslator::OpString RefTranslator::DoGetString(KxStandardID id) const
	{
		bool isSuccess = false;
		wxString value = m_TranslationRef.GetString(id, &isSuccess);
		if (isSuccess)
		{
			return value;
		}
		return std::nullopt;
	}
}
