#pragma once
#include "stdafx.h"
#include "KModEntry.h"
#include "KModManagerTags.h"

class KMMModelNode
{
	enum Type
	{
		None = -1,
		Group,
		Entry,
	};
	typedef std::vector<KMMModelNode> NodeVector;
	typedef std::vector<KMMModelNode*> NodeRefVector;

	private:
		Type m_Type = None;
		union
		{
			KModEntry* m_Entry = NULL;
			const KModTag* m_Group;
		};
		const KMMModelNode* m_ParentNode = NULL;
		NodeVector m_Children;

	public:
		KMMModelNode()
		{
		}
		KMMModelNode(const KModTag& group)
			:m_Group(&group), m_Type(Group)
		{
		}
		KMMModelNode(KModEntry* entry)
			:m_Entry(entry), m_Type(Entry)
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

		const KModTag* GetGroup() const
		{
			return IsGroup() ? m_Group : NULL;
		}
		KModEntry* GetEntry() const
		{
			return IsEntry() ? m_Entry : NULL;
		}

		bool HasParentNode() const
		{
			return m_ParentNode != NULL;
		}
		const KMMModelNode* GetParentNode() const
		{
			return m_ParentNode;
		}
		void SetParentNode(const KMMModelNode& node)
		{
			m_ParentNode = &node;
		}

		bool HasChildren() const
		{
			return !m_Children.empty();
		}
		const NodeVector& GetChildren() const
		{
			return m_Children;
		}
		NodeVector& GetChildren()
		{
			return m_Children;
		}
};
typedef std::vector<KMMModelNode> KMMLogModelNodeVector;
typedef std::vector<KMMModelNode*> KMMLogModelNodeRefVector;
