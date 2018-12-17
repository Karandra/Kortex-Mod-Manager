#pragma once
#include "stdafx.h"
#include "GameMods/IModTagManager.h"

namespace Kortex::ModTagManager
{
	class DefaultTagManager: public IModTagManager
	{
		private:
			IModTag::Vector m_Tags;

		protected:
			void LoadTagsFromFile(const wxString& filePath, bool markAsSystem);
			void SaveTagsToFile(const wxString& filePath) const;

			void LoadDefaultTags();
			void LoadUserTags();
			void SaveUserTags() const;

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;

		public:
			const IModTag::Vector& GetTags() const override
			{
				return m_Tags;
			}
			IModTag::Vector& GetTags() override
			{
				return m_Tags;
			}
	
			void LoadTagsFromMod(const IGameMod& mod) override;
			std::unique_ptr<IModTag> NewTag() override;
	};
}
