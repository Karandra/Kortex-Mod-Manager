#include "stdafx.h"
#include "IModTagManager.h"
#include "GameMods/GameModsModule.h"
#include <KxFramework/KxUtility.h>

namespace
{
	enum class FindBy
	{
		Object,
		ID,
		Name
	};

	template<class TagT, FindBy findBy, class VectorT, class ValueT>
	TagT* FindModTag(VectorT& tags, ValueT&& value, typename VectorT::const_iterator* iterator = nullptr)
	{
		auto it = std::find_if(tags.begin(), tags.end(), [&value](const auto& tag)
		{
			if constexpr(findBy == FindBy::Object)
			{
				return tag.get() == &value;
			}
			else if constexpr(findBy == FindBy::ID)
			{
				return tag->GetID() == value;
			}
			else if constexpr(findBy == FindBy::Name)
			{
				return tag->GetName() == value;
			}
			return false;
		});

		if (it != tags.end())
		{
			KxUtility::SetIfNotNull(iterator, it);
			return (*it).get();
		}
		else
		{
			KxUtility::SetIfNotNull(iterator, tags.end());
			return nullptr;
		}
	}
}

namespace Kortex
{
	namespace ModTagManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ModTagManager", "ModTagManager.Name");
	}

	IModTagManager::IModTagManager()
		:ManagerWithTypeInfo(GameModsModule::GetInstance())
	{
	}

	IModTag& IModTagManager::EmplaceTag()
	{
		return EmplaceTag(NewTag());
	}
	IModTag& IModTagManager::EmplaceTag(std::unique_ptr<IModTag> tag)
	{
		IModTag::Vector& tags = GetTags();
		IModTag* existingTag = FindModTag<IModTag, FindBy::Object>(tags, *tag);

		if (existingTag)
		{
			return *existingTag;
		}
		return *tags.emplace_back(std::move(tag));
	}
	IModTag& IModTagManager::EmplaceTagWith(const wxString& id, const wxString& name)
	{
		IModTag* existingTag = FindTagByID(id);
		if (existingTag)
		{
			return *existingTag;
		}
		else
		{
			IModTag& tag = EmplaceTag();
			tag.SetID(id);
			tag.SetName(name.IsEmpty() ? id : name);

			return tag;
		}
	}

	void IModTagManager::RemoveAllTags()
	{
		GetTags().clear();
	}
	bool IModTagManager::RemoveTag(IModTag& tag)
	{
		IModTag::Vector& tags = GetTags();
		IModTag::Vector::const_iterator it;

		if (FindModTag<IModTag, FindBy::Object>(tags, tag, &it))
		{
			tags.erase(it);
			return true;
		}
		return false;
	}
	bool IModTagManager::RemoveTagByID(const wxString& id)
	{
		IModTag::Vector& tags = GetTags();
		IModTag::Vector::const_iterator it;

		if (FindModTag<IModTag, FindBy::ID>(tags, id, &it))
		{
			tags.erase(it);
			return true;
		}
		return false;
	}

	IModTag* IModTagManager::FindTagByID(const wxString& id) const
	{
		return FindModTag<IModTag, FindBy::ID>(GetTags(), id);
	}
	IModTag* IModTagManager::FindTagByName(const wxString& name) const
	{
		return FindModTag<IModTag, FindBy::Name>(GetTags(), name);
	}
	wxString IModTagManager::GetTagNameByID(const wxString& id) const
	{
		if (const IModTag* tag = FindTagByID(id))
		{
			return tag->GetName();
		}
		return wxEmptyString;
	}
}
