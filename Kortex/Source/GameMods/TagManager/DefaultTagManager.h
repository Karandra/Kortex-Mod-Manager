#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"
#include "GameMods/IModTagManager.h"
class KxXMLNode;

namespace Kortex
{
	class IGameProfile;
	class ProfileEvent;
}

namespace Kortex::ModTagManager
{
	class DefaultTagManager: public IModTagManager
	{
		private:
			BroadcastReciever m_BroadcastReciever;

			IModTag::Vector m_DefaultTags;
			IModTag::Vector m_UserTags;

		private:
			void OnProfileSelected(ProfileEvent& event);

		protected:
			void LoadTagsFrom(IModTag::Vector& items, const KxXMLNode& tagsNode);
			void SaveTagsTo(const IModTag::Vector& items, KxXMLNode& tagsNode) const;

			void LoadUserTags(const IGameProfile& profile);
			void SaveUserTags(IGameProfile& profile) const;
			void ClearUserTags();

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;

		public:
			const IModTag::Vector& GetDefaultTags() const override
			{
				return m_DefaultTags;
			}
			const IModTag::Vector& GetTags() const override
			{
				return m_UserTags;
			}
			IModTag::Vector& GetTags() override
			{
				return m_UserTags;
			}
	
			void LoadTagsFromMod(IModTag::Vector& items, const IGameMod& mod) override;
			std::unique_ptr<IModTag> NewTag() override;
	};
}
