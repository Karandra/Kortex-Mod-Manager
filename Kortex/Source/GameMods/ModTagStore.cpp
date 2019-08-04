#include "stdafx.h"
#include "ModTagStore.h"
#include <Kortex/ModTagManager.hpp>

namespace Kortex
{
	class ModTagStoreTag: public IModTag
	{
		private:
			wxString m_ID;

		public:
			ModTagStoreTag(const wxString& id)
				:m_ID(id)
			{
			}

		public:
			bool IsOK() const override
			{
				return !m_ID.IsEmpty();
			}
			std::unique_ptr<IModTag> Clone() const override
			{
				return nullptr;
			}
			
			bool IsExpanded() const override
			{
				return false;
			}
			void SetExpanded(bool isExpanded) override
			{
			}

			wxString GetID() const override
			{
				return m_ID;
			}
			void SetID(const wxString& value) override
			{
				m_ID = value;
			}

			wxString GetName() const override
			{
				if (auto name = GetTranslatedNameByID(m_ID))
				{
					return *name;
				}
				return m_ID;
			}
			void SetName(const wxString& label) override
			{
				m_ID = label;
			}

			KxColor GetColor() const override
			{
				return {};
			}
			void SetColor(const KxColor& color) override
			{
			}
	};
}

namespace Kortex
{
	template<class MapT, class Functor> void VisitHelper(MapT&& map, Functor&& functor)
	{
		IModTagManager* manager = IModTagManager::GetInstance();

		for (const wxString& tagID: map)
		{
			if (IModTag* realTag = manager->FindTagByID(tagID))
			{
				if (!functor(*realTag))
				{
					break;
				}
			}
			else
			{
				ModTagStoreTag tag(tagID);
				if (!functor(tag))
				{
					break;
				}
			}
		}
	}
}

namespace Kortex
{
	bool ModTagStore::HasTag(const IModTag& tag) const
	{
		if (tag.IsOK())
		{
			return HasTag(tag.GetID());
		}
		return false;
	}
	void ModTagStore::AddTag(const IModTag& tag)
	{
		if (tag.IsOK())
		{
			AddTag(tag.GetID());
		}
	}
	void ModTagStore::RemoveTag(const IModTag& tag)
	{
		if (tag.IsOK())
		{
			RemoveTag(tag.GetID());
		}
	}
	void ModTagStore::ToggleTag(const IModTag& tag, bool addTag)
	{
		if (tag.IsOK())
		{
			addTag ? AddTag(tag) : RemoveTag(tag);
		}
	}

	void ModTagStore::Visit(const Visitor& visitor)
	{
		VisitHelper(m_TagIDs, visitor);
	}
	void ModTagStore::Visit(const CVisitor& visitor) const
	{
		VisitHelper(m_TagIDs, visitor);
	}

	KxStringVector ModTagStore::GetIDs() const
	{
		KxStringVector values;
		for (const wxString& tagID: m_TagIDs)
		{
			values.push_back(tagID);
		}
		return values;
	}
	KxStringVector ModTagStore::GetNames() const
	{
		KxStringVector values;
		VisitHelper(m_TagIDs, [&values](const IModTag& tag)
		{
			values.push_back(tag.GetName());
			return true;
		});
		return values;
	}
}
