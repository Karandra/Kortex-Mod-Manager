#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IGameMod;
}

namespace Kortex::ModManager
{
	class DisplayModelDNDObject: public wxDataObjectSimple
	{
		public:
			static wxDataFormat GetFormat()
			{
				return typeid(DisplayModel).name();
			}

		private:
			std::vector<IGameMod*> m_Mods;

		public:
			DisplayModelDNDObject(size_t count = 0)
				:wxDataObjectSimple(GetFormat())
			{
				m_Mods.reserve(count);
			}

		public:
			bool IsEmpty() const
			{
				return m_Mods.empty();
			}

			void Clear()
			{
				m_Mods.clear();
			}
			void AddMod(IGameMod& mod)
			{
				m_Mods.push_back(&mod);
			}
			auto& GetMods()
			{
				return m_Mods;
			}
	};
}
