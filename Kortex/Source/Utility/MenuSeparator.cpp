#include "stdafx.h"
#include "Utility/MenuSeparator.h"
#include <KxFramework/KxMenu.h>

namespace Kortex::Utility
{
	MenuSeparator::MenuSeparator(KxMenu& menu, Where insertWhere)
		:m_Menu(menu), m_ItemCount(menu.GetMenuItemCount()), m_Where(insertWhere)
	{
	}
	MenuSeparator::~MenuSeparator()
	{
		if (m_Menu.GetMenuItemCount() > m_ItemCount)
		{
			switch (m_Where)
			{
				case Where::Before:
				{
					m_Menu.InsertSeparator(m_ItemCount);
					break;
				}
				case Where::After:
				{
					m_Menu.AddSeparator();
					break;
				}
			};
		}
	}
}
