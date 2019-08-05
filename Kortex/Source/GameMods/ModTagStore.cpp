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
	bool ModTagStore::HasTag(const wxString& tagID) const
	{
		if (!tagID.IsEmpty())
		{
			return m_TagIDs.find(tagID) != m_TagIDs.end();
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
	void ModTagStore::AddTag(const wxString& tagID)
	{
		if (!tagID.IsEmpty())
		{
			m_TagIDs.insert(tagID);
		}
	}

	void ModTagStore::RemoveTag(const IModTag& tag)
	{
		if (tag.IsOK())
		{
			RemoveTag(tag.GetID());
		}
	}
	void ModTagStore::RemoveTag(const wxString& tagID)
	{
		if (!tagID.IsEmpty())
		{
			m_TagIDs.erase(tagID);
		}
	}

	void ModTagStore::ToggleTag(const IModTag& tag, bool addTag)
	{
		if (tag.IsOK())
		{
			addTag ? AddTag(tag) : RemoveTag(tag);
		}
	}
	void ModTagStore::ToggleTag(const wxString& tagID, bool addTag)
	{
		if (!tagID.IsEmpty())
		{
			addTag ? AddTag(tagID) : RemoveTag(tagID);
		}
	}

	void ModTagStore::Clear()
	{
		m_TagIDs.clear();
		ClearPrimaryTag();
	}
	void ModTagStore::Visit(const Visitor& visitor)
	{
		VisitHelper(m_TagIDs, visitor);
	}
	void ModTagStore::Visit(const CVisitor& visitor) const
	{
		VisitHelper(m_TagIDs, visitor);
	}

	IModTag* ModTagStore::GetPrimaryTag() const
	{
		if (!m_PrimaryTag.IsEmpty())
		{
			return IModTagManager::GetInstance()->FindTagByID(m_PrimaryTag);
		}
		return nullptr;
	}
	wxString ModTagStore::GetPrimaryTagID() const
	{
		return m_PrimaryTag;
	}
	bool ModTagStore::SetPrimaryTag(const IModTag& tag)
	{
		ClearPrimaryTag();
		if (tag.IsOK())
		{
			if (!HasTag(tag))
			{
				AddTag(tag);
			}
			m_PrimaryTag = tag.GetID();

			return true;
		}
		return false;
	}
	bool ModTagStore::SetPrimaryTag(const wxString& tagID)
	{
		if (tagID.IsEmpty())
		{
			return ClearPrimaryTag();
		}
		else if (const IModTag* tag = IModTagManager::GetInstance()->FindTagByID(tagID))
		{
			return SetPrimaryTag(*tag);
		}
		return false;
	}
	bool ModTagStore::ClearPrimaryTag()
	{
		if (!m_PrimaryTag.IsEmpty())
		{
			m_PrimaryTag.clear();
			return true;
		}
		return false;
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
