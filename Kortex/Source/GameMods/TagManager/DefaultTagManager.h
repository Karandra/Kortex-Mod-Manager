#pragma once
#include "stdafx.h"
#include "GameMods/IModTagManager.h"
class KxXMLNode;

namespace Kortex::ModTagManager
{
	class DefaultTagManager: public IModTagManager
	{
		private:
			IModTag::Vector m_DefaultTags;
			IModTag::Vector m_UserTags;

		protected:
			void LoadTagsFrom(IModTag::Vector& items, const KxXMLNode& tagsNode);
			void SaveTagsTo(const IModTag::Vector& items, KxXMLNode& tagsNode) const;

			void LoadUserTags();
			void SaveUserTags() const;

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
