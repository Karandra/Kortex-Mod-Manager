#pragma once
#include "stdafx.h"
#include "PackageProject/KPackageProjectComponents.h"

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ComponentsModelNode
	{
		public:
			using Vector = std::vector<std::unique_ptr<ComponentsModelNode>>;
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
				ConditionFlags,
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
			ComponentsModelNode* m_Parent = nullptr;
			Vector m_Children;
	
		public:
			ComponentsModelNode(KPPCStep* value)
				:m_Value(value)
			{
			}
			ComponentsModelNode(KPPCGroup* value, ComponentsModelNode* parent)
				:m_Value(value), m_Parent(parent)
			{
			}
			ComponentsModelNode(KPPCEntry* value, ComponentsModelNode* parent)
				:m_Value(value), m_Parent(parent)
			{
			}
			ComponentsModelNode(EntryID id, ComponentsModelNode* parent)
				:m_Value(id), m_Parent(parent)
			{
			}
	
		public:
			KPPCStep* GetStep() const
			{
				auto value = std::get_if<KPPCStep*>(&m_Value);
				return value ? *value : nullptr;
			}
			KPPCGroup* GetGroup() const
			{
				auto value = std::get_if<KPPCGroup*>(&m_Value);
				return value ? *value : nullptr;
			}
			KPPCEntry* GetEntry() const
			{
				auto value = std::get_if<KPPCEntry*>(&m_Value);
				return value ? *value : nullptr;
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
			ComponentsModelNode* AddEntryItem(EntryID id)
			{
				return m_Children.emplace_back(std::make_unique<ComponentsModelNode>(id, this)).get();
			}
			void CreateFullEntryNode()
			{
				AddEntryItem(EntryID::TypeDescriptor);
				AddEntryItem(EntryID::FileData);
				AddEntryItem(EntryID::Requirements);
				AddEntryItem(EntryID::Image);
				AddEntryItem(EntryID::Description);
				AddEntryItem(EntryID::Conditions);
				AddEntryItem(EntryID::ConditionFlags);
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
				return m_Parent != nullptr;
			}
			ComponentsModelNode* GetParent() const
			{
				return m_Parent;
			}
			void SetParent(ComponentsModelNode* node)
			{
				m_Parent = node;
			}
	
		public:
			bool IsSameType(const ComponentsModelNode* other) const
			{
				return m_Value.index() == other->m_Value.index();
			}
			bool IsSameBranch(const ComponentsModelNode* other) const
			{
				return m_Parent == other->GetParent();
			}
			
			bool operator==(const ComponentsModelNode& other) const
			{
				return m_Value == other.m_Value && m_Parent == other.m_Parent && m_Children.size() == other.m_Children.size();
			}
			bool operator!=(const ComponentsModelNode& other) const
			{
				return !(*this == other);
			}
	};
}
