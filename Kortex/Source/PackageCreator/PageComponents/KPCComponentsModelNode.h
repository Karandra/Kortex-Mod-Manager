#pragma once
#include "stdafx.h"
#include "PackageProject/KPackageProjectComponents.h"

class KPCComponentsModelNode
{
	public:
		using Vector = std::vector<std::unique_ptr<KPCComponentsModelNode>>;
		enum EntryID: int
		{
			InvalidEntryID = -1,

			TypeDescriptor,
			Name,
			FileData,
			Requirements,
			Image,
			Description,
			Conditions,
			AssignedFlags,
		};

	private:
		enum Type
		{
			None = -1,
			Step,
			Group,
			Entry,
			EntryValue,
		};

	private:
		std::variant<KPPCStep*, KPPCGroup*, KPPCEntry*, EntryID> m_Value;
		KPCComponentsModelNode* m_Parent = NULL;
		Vector m_Children;

	public:
		KPCComponentsModelNode(KPPCStep* value)
			:m_Value(value)
		{
		}
		KPCComponentsModelNode(KPPCGroup* value, KPCComponentsModelNode* parent)
			:m_Value(value), m_Parent(parent)
		{
		}
		KPCComponentsModelNode(KPPCEntry* value, KPCComponentsModelNode* parent)
			:m_Value(value), m_Parent(parent)
		{
		}
		KPCComponentsModelNode(EntryID id, KPCComponentsModelNode* parent)
			:m_Value(id), m_Parent(parent)
		{
		}

	public:
		KPPCStep* GetStep() const
		{
			auto value = std::get_if<KPPCStep*>(&m_Value);
			return value ? *value : NULL;
		}
		KPPCGroup* GetGroup() const
		{
			auto value = std::get_if<KPPCGroup*>(&m_Value);
			return value ? *value : NULL;
		}
		KPPCEntry* GetEntry() const
		{
			auto value = std::get_if<KPPCEntry*>(&m_Value);
			return value ? *value : NULL;
		}

		bool IsEntryItem() const
		{
			return m_Value.index() == Type::EntryValue;
		}
		EntryID GetEntryItemID() const
		{
			auto value = std::get_if<EntryID>(&m_Value);
			return value ? *value : EntryID::InvalidEntryID;
		}
		KPCComponentsModelNode* AddEntryItem(EntryID id)
		{
			return m_Children.emplace_back(new KPCComponentsModelNode(id, this)).get();
		}
		void CreateFullEntryNode()
		{
			AddEntryItem(EntryID::TypeDescriptor);
			AddEntryItem(EntryID::FileData);
			AddEntryItem(EntryID::Requirements);
			AddEntryItem(EntryID::Image);
			AddEntryItem(EntryID::Description);
			AddEntryItem(EntryID::Conditions);
			AddEntryItem(EntryID::AssignedFlags);
		}

		bool HasChildren() const
		{
			return !m_Children.empty();
		}
		const Vector& GetChildren() const
		{
			return m_Children;
		}
		Vector& GetChildren()
		{
			return m_Children;
		}

		bool HasParent() const
		{
			return m_Parent != NULL;
		}
		KPCComponentsModelNode* GetParent() const
		{
			return m_Parent;
		}

	public:
		bool IsSameType(const KPCComponentsModelNode* other) const
		{
			return m_Value.index() == other->m_Value.index();
		}
		bool operator==(const KPCComponentsModelNode& other) const
		{
			return m_Value == other.m_Value && m_Parent == other.m_Parent && m_Children.size() == other.m_Children.size();
		}
		bool operator!=(const KPCComponentsModelNode& other) const
		{
			return !(*this == other);
		}
};
