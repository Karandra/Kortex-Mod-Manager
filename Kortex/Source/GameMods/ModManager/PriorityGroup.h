#pragma once
#include "stdafx.h"
#include "FixedGameMod.h"
#include "Utility/KImageProvider.h"

namespace Kortex
{
	class IGameMod;
	class IModTag;
}

namespace Kortex::ModManager
{
	class IPriorityGroup: public RTTI::IInterface<IPriorityGroup>
	{
		public:
			virtual IGameMod& GetBaseMod() const = 0;
			virtual IModTag* GetTag() const = 0;
			virtual void SetTag(IModTag* tag) = 0;

			virtual bool IsBegin() const = 0;
			virtual bool IsEnd() const = 0;
	};

	class PriorityGroup: public RTTI::IExtendInterface<PriorityGroup, FixedGameMod, IPriorityGroup>
	{
		public:
			IGameMod& m_BaseMod;
			IModTag* m_Tag = nullptr;
			bool m_IsBegin = false;

		public:
			PriorityGroup(IGameMod& baseMod, bool isBegin)
				:m_BaseMod(baseMod), m_IsBegin(isBegin)
			{
			}

		public:
			bool IsInstalled() const override
			{
				return false;
			}
			bool IsLinkedMod() const override
			{
				return false;
			}

			KImageEnum GetIcon() const override
			{
				return KIMG_NONE;
			}
			intptr_t GetPriority() const override
			{
				return m_BaseMod.GetPriority();
			}
			intptr_t GetOrderIndex() const override
			{
				return m_BaseMod.GetOrderIndex() + (m_IsBegin ? -1 : +1);
			}

			bool HasColor() const override;
			KxColor GetColor() const override;
			void SetColor(const KxColor& color) override;

			IGameMod& GetBaseMod() const override
			{
				return m_BaseMod;
			}
			IModTag* GetTag() const override
			{
				return m_Tag;
			}
			void SetTag(IModTag* tag) override
			{
				m_Tag = tag;
			}

			bool IsBegin() const override
			{
				return m_IsBegin;
			}
			bool IsEnd() const override
			{
				return !m_IsBegin;
			}
	};
}
