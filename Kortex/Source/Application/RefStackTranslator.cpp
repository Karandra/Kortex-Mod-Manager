#include "stdafx.h"
#include "RefStackTranslator.h"
#include <KxFramework/KxTranslation.h>

namespace
{
	using namespace Kortex;

	template<class T> ITranslator::OpString GetTranslationFor(const RefStackTranslator& translator, const T& id)
	{
		wxString value;
		bool isSuccess = false;
		translator.ForEachTranslation([&id, &value, &isSuccess](const KxTranslation& translation)
		{
			value = translation.GetString(id, &isSuccess);
			if (isSuccess)
			{
				return false;
			}
			return true;
		});
		if (isSuccess)
		{
			return value;
		}
		return std::nullopt;
	}
}

namespace Kortex
{
	ITranslator::OpString RefStackTranslator::DoGetString(const wxString& id) const
	{
		return GetTranslationFor(*this, id);
	}
	ITranslator::OpString RefStackTranslator::DoGetString(KxStandardID id) const
	{
		return GetTranslationFor(*this, id);
	}
}
