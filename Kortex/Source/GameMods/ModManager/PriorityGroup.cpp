#include "stdafx.h"
#include "PriorityGroup.h"
#include <Kortex/ModTagManager.hpp>
#include "KImageProvider.h"

namespace Kortex::ModManager
{
	bool PriorityGroup::HasColor() const
	{
		return m_Tag && m_Tag->HasColor();
	}
	KxColor PriorityGroup::GetColor() const
	{
		return m_Tag ? m_Tag->GetColor() : KxColor();
	}
	void PriorityGroup::SetColor(const KxColor& color)
	{
		if (m_Tag)
		{
			m_Tag->SetColor(color);
		}
	}
}
