#pragma once
#include "stdafx.h"
#include "IModTag.h"
#include "Application/IManager.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	namespace ModTagManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IGameMod;

	class IModTagManager:
		public ManagerWithTypeInfo<IManager, SimpleManagerInfo, ModTagManager::Internal::TypeInfo>,
		public KxSingletonPtr<IModTagManager>
	{
		public:
			IModTagManager();

		public:
			virtual const IModTag::Vector& GetDefaultTags() const = 0;
			virtual const IModTag::Vector& GetTags() const = 0;
			virtual IModTag::Vector& GetTags() = 0;
			size_t GetTagsCount() const
			{
				return GetTags().size();
			}
			bool HasTags() const
			{
				return !GetTags().empty();
			}

			virtual void LoadTagsFromMod(IModTag::Vector& items, const IGameMod& mod) = 0;
			void LoadTagsFromMod(const IGameMod& mod)
			{
				LoadTagsFromMod(GetTags(), mod);
			}
			
			void LoadDefaultTags(IModTag::Vector& items);
			void LoadDefaultTags();

			virtual std::unique_ptr<IModTag> NewTag() = 0;

			IModTag& EmplaceTag();
			IModTag& EmplaceTag(IModTag::Vector& items);

			IModTag& EmplaceTag(std::unique_ptr<IModTag> tag);
			IModTag& EmplaceTag(IModTag::Vector& items, std::unique_ptr<IModTag> tag);

			IModTag& EmplaceTagWith(const wxString& id, const wxString& name = wxEmptyString);
			IModTag& EmplaceTagWith(IModTag::Vector& items, const wxString& id, const wxString& name = wxEmptyString);

			void RemoveAllTags();
			void RemoveAllTags(IModTag::Vector& items);

			bool RemoveTag(IModTag& tag);
			bool RemoveTag(IModTag::Vector& items, IModTag& tag);

			bool RemoveTagByID(const wxString& id);
			bool RemoveTagByID(IModTag::Vector& items, const wxString& id);

			IModTag* FindTagByID(const wxString& id) const;
			IModTag* FindTagByID(const IModTag::Vector& items, const wxString& id) const;

			IModTag* FindTagByName(const wxString& name) const;
			IModTag* FindTagByName(const IModTag::Vector& items, const wxString& name) const;

			wxString GetTagNameByID(const wxString& id) const;
			wxString GetTagNameByID(const IModTag::Vector& items, const wxString& id) const;
	};
}
