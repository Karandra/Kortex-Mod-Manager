#pragma once
#include "stdafx.h"
#include "BasicGameMod.h"
#include "KImageProvider.h"

namespace Kortex::ModManager
{
	class IFixedGameMod: public RTTI::IInterface<IFixedGameMod>
	{
	};

	class FixedGameMod: public RTTI::IImplementation<BasicGameMod, IFixedGameMod>
	{
		private:
			intptr_t m_Priority = -1;

		public:
			FixedGameMod(intptr_t priority = -1)
				:m_Priority(priority)
			{
			}

		public:
			virtual bool LoadUsingSignature(const wxString& signature) override
			{
				return false;
			}
			virtual bool Save() override
			{
				return false;
			}

		public:
			virtual bool IsActive() const override
			{
				return true;
			}
			virtual bool IsInstalled() const override
			{
				return true;
			}

			virtual KImageEnum GetIcon() const override
			{
				return KIMG_FOLDERS;
			}
			virtual intptr_t GetPriority() const override
			{
				return m_Priority;
			}
			virtual intptr_t GetOrderIndex() const override
			{
				return m_Priority;
			}
	};
}
