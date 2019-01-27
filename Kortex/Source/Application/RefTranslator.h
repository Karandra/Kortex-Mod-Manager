#pragma once
#include "stdafx.h"
#include "ITranslator.h"
class KxTranslation;

namespace Kortex
{
	class RefTranslator: public ITranslator
	{
		private:
			const KxTranslation& m_TranslationRef;

		protected:
			OpString DoGetString(const wxString& id) const override;
			OpString DoGetString(KxStandardID id) const override;
			
		public:
			RefTranslator(const KxTranslation& translation)
				:m_TranslationRef(translation)
			{
			}

		public:
			const KxTranslation& GetTranslation() const
			{
				return m_TranslationRef;
			}
	};
}
