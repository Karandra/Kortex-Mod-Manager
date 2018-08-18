#pragma once
#include "stdafx.h"
#include "KInstallWizardDefs.h"

class KIWCModelNode
{
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
			const KPPCGroup* m_Group = NULL;
			const KPPCEntry* m_Entry;
		};
		const KIWCModelNode* m_ParentNode = NULL;

		size_t m_Begin = 0;
		size_t m_Size = 0;
		wxCheckBoxState m_CheckState = wxCHK_UNDETERMINED;

	public:
		KIWCModelNode()
		{
		}
		KIWCModelNode(const KPPCGroup& group)
			:m_Group(&group), m_Type(Group)
		{
		}
		KIWCModelNode(const KPPCEntry& entry)
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

		const KPPCGroup* GetGroup() const
		{
			return IsGroup() ? m_Group : NULL;
		}
		const KPPCEntry* GetEntry() const
		{
			return IsEntry() ? m_Entry : NULL;
		}

		bool HasParentNode() const
		{
			return m_ParentNode != NULL;
		}
		const KIWCModelNode* GetParentNode() const
		{
			return m_ParentNode;
		}
		void SetParentNode(const KIWCModelNode& node)
		{
			m_ParentNode = &node;
		}

		size_t GetBegin() const
		{
			return m_Begin;
		}
		size_t GetSize() const
		{
			return m_Size;
		}
		bool HasChildren() const
		{
			return m_Begin < m_Size;
		}
		void SetBounds(size_t nBegin, size_t nSize)
		{
			m_Begin = nBegin;
			m_Size = nSize;
		}

		bool IsRequiredEntry() const
		{
			if (const KPPCEntry* entry = GetEntry())
			{
				return entry->GetTDCurrentValue() == KPPC_DESCRIPTOR_REQUIRED;
			}
			return false;
		}
		bool IsChecked() const
		{
			return m_CheckState == wxCHK_CHECKED || IsRequiredEntry();
		}
		void SetChecked(bool bCheck)
		{
			m_CheckState = bCheck || IsRequiredEntry() ? wxCHK_CHECKED : wxCHK_UNCHECKED;
		}
		void ToggleCheck()
		{
			SetChecked(!IsChecked());
		}
};
typedef std::vector<KIWCModelNode> KIWCNodesVector;
typedef std::vector<KIWCModelNode*> KIWCNodesRefVector;
