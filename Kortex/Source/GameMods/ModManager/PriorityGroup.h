#pragma once
#include "stdafx.h"
#include "BasicGameMod.h"
#include "Utility/KImageProvider.h"

namespace Kortex
{
	class IGameMod;
	class IModTag;
}

namespace Kortex::ModManager
{
	class PriorityGroup: public RTTI::IExtendInterface<PriorityGroup, BasicGameMod>
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
