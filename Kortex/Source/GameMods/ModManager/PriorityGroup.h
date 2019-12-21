#pragma once
#include "stdafx.h"
#include "BasicGameMod.h"

namespace Kortex
{
	class IGameMod;
	class IModTag;
}

namespace Kortex::ModManager
{
	class PriorityGroup: public KxRTTI::ExtendInterface<PriorityGroup, BasicGameMod>
	{
		KxDecalreIID(PriorityGroup, {0x23b5fc04, 0xc2f5, 0x42fb, {0xa6, 0x91, 0x45, 0xd8, 0x78, 0x37, 0xc9, 0xf4}});

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
			bool IsActive() const override
			{
				return false;
			}

			intptr_t GetDisplayOrder() const override
			{
				return m_BaseMod.GetDisplayOrder();
			}
			wxString GetName() const override;
			wxString GetID() const override;

			bool HasColor() const override;
			KxColor GetColor() const override;
			void SetColor(const KxColor& color) override;

			IGameMod& GetBaseMod() const
			{
				return m_BaseMod;
			}
			IModTag* GetTag() const
			{
				return m_Tag;
			}
			void SetTag(IModTag* tag)
			{
				m_Tag = tag;
			}

			bool IsBegin() const
			{
				return m_IsBegin;
			}
			bool IsEnd() const
			{
				return !m_IsBegin;
			}
	};
}
