#pragma once
#include "stdafx.h"
#include "BasicGameMod.h"

namespace Kortex::ModManager
{
	class FixedGameMod: public KxRTTI::ExtendInterface<FixedGameMod, BasicGameMod>
	{
		private:
			const intptr_t m_FixedOrder = -1;

		public:
			FixedGameMod(intptr_t order = -1)
				:m_FixedOrder(order)
			{
			}

		public:
			bool LoadUsingSignature(const wxString& signature) override
			{
				return false;
			}
			bool Save() override
			{
				return false;
			}

		public:
			bool IsActive() const override
			{
				return true;
			}
			bool IsInstalled() const override
			{
				return true;
			}

			ResourceID GetIcon() const override
			{
				return ImageResourceID::Folders;
			}
			intptr_t GetDisplayOrder() const override
			{
				return m_FixedOrder;
			}
	};
}
