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

			virtual void LoadTagsFromMod(const IGameMod& mod) = 0;

			virtual std::unique_ptr<IModTag> NewTag() = 0;
			IModTag& EmplaceTag();
			IModTag& EmplaceTag(std::unique_ptr<IModTag> tag);
			IModTag& EmplaceTagWith(const wxString& id, const wxString& name = wxEmptyString);

			void RemoveAllTags();
			bool RemoveTag(IModTag& tag);
			bool RemoveTagByID(const wxString& id);

			IModTag* FindTagByID(const wxString& id) const;
			IModTag* FindTagByName(const wxString& name) const;
			wxString GetTagNameByID(const wxString& id) const;
	};
}
