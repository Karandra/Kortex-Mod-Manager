#pragma once
#include "stdafx.h"
#include "ITranslator.h"
class KxTranslation;

namespace Kortex
{
	class RefStackTranslator: public ITranslator
	{
		private:
			std::vector<std::reference_wrapper<const KxTranslation>> m_Translations;

		protected:
			OpString DoGetString(const wxString& id) const override;
			OpString DoGetString(KxStandardID id) const override;
			
		public:
			RefStackTranslator() = default;
			RefStackTranslator(const KxTranslation& translation)
			{
				m_Translations.push_back(std::cref(translation));
			}

		public:
			void Push(const KxTranslation& translation)
			{
				m_Translations.push_back(std::cref(translation));
			}
			void Pop()
			{
				m_Translations.pop_back();
			}
			
			template<class TFunctor> void ForEachTranslation(TFunctor&& func) const
			{
				for (auto it = m_Translations.rbegin(); it != m_Translations.rend(); ++it)
				{
					if (!func(*it))
					{
						return;
					}
				}
			}
	};
}
