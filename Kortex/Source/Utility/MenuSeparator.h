#pragma once
#include "stdafx.h"
class KxMenu;

namespace Kortex::Utility
{
	class MenuSeparator
	{
		public:
			enum class Where
			{
				Before,
				After,
			};

		private:
			KxMenu& m_Menu;
			const size_t m_ItemCount = 0;
			const Where m_Where = Where::After;

		public:
			MenuSeparator(KxMenu& menu, Where insertWhere);
			MenuSeparator(const MenuSeparator&) = delete;
			~MenuSeparator();

		public:
			MenuSeparator& operator=(const MenuSeparator&) = delete;
	};
}

namespace Kortex::Utility
{
	class MenuSeparatorBefore: public MenuSeparator
	{
		public:
			MenuSeparatorBefore(KxMenu& menu)
				:MenuSeparator(menu, Where::Before)
			{
			}
	};

	class MenuSeparatorAfter: public MenuSeparator
	{
		public:
			MenuSeparatorAfter(KxMenu& menu)
				:MenuSeparator(menu, Where::After)
			{
			}
	};
}
