#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IGameMod;
	class IModTag;
}

namespace Kortex::ModManager
{
	class DisplayModelNode
	{
		public:
			using Vector = std::vector<DisplayModelNode>;
			using RefVector = std::vector<DisplayModelNode*>;

		private:
			enum Type
			{
				None = -1,
				Group,
				Entry,
			};

		private:
			Type m_Type = None;
			union
			{
				IGameMod* m_Entry = nullptr;
				const IModTag* m_Group;
			};
			const DisplayModelNode* m_ParentNode = nullptr;
			Vector m_Children;

		public:
			DisplayModelNode()
			{
			}
			DisplayModelNode(const IModTag& group)
				:m_Group(&group), m_Type(Group)
			{
			}
			DisplayModelNode(IGameMod& entry)
				:m_Entry(&entry), m_Type(Entry)
			{
			}

		public:
			bool IsOK() const
			{
				return m_Type != None;
			}
			bool IsGroup() const
			{
				return m_Type == Group;
			}
			bool IsEntry() const
			{
				return m_Type == Entry;
			}

			const IModTag* GetGroup() const
			{
				return IsGroup() ? m_Group : nullptr;
			}
			IGameMod* GetEntry() const
			{
				return IsEntry() ? m_Entry : nullptr;
			}

			bool HasParentNode() const
			{
				return m_ParentNode != nullptr;
			}
			const DisplayModelNode* GetParentNode() const
			{
				return m_ParentNode;
			}
			void SetParentNode(const DisplayModelNode& node)
			{
				m_ParentNode = &node;
			}

			bool HasChildren() const
			{
				return !m_Children.empty();
			}
			size_t GetChildrenCount() const
			{
				return m_Children.size();
			}
			const Vector& GetChildren() const
			{
				return m_Children;
			}
			Vector& GetChildren()
			{
				return m_Children;
			}
	};
}
