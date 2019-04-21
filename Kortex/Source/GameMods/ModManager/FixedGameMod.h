#pragma once
#include "stdafx.h"
#include "BasicGameMod.h"
#include "Utility/KImageProvider.h"

namespace Kortex::ModManager
{
	class FixedGameMod: public RTTI::IExtendInterface<FixedGameMod, BasicGameMod>
	{
		private:
			intptr_t m_FixedPriority = -1;

		public:
			FixedGameMod(intptr_t priority = -1)
				:m_FixedPriority(priority)
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

			KImageEnum GetIcon() const override
			{
				return KIMG_FOLDERS;
			}
			intptr_t GetPriority() const override
			{
				return m_FixedPriority;
			}
			void SetPriority(intptr_t value) override
			{
			}
	};
}
