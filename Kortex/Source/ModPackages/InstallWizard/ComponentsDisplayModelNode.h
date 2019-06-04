#pragma once
#include "stdafx.h"
#include "Common.h"

namespace Kortex::InstallWizard
{
	class ComponentsDisplayModelNode
	{
		public:
			using Vector = std::vector<ComponentsDisplayModelNode>;
			using RefVector = std::vector<ComponentsDisplayModelNode*>;

		private:
			enum Type
			{
				None = -1,
				Group,
				Entry,
			};

		private:
			union
			{
				const KPPCGroup* m_Group = nullptr;
				const KPPCEntry* m_Entry;
			};
			const ComponentsDisplayModelNode* m_ParentNode = nullptr;

			size_t m_Begin = 0;
			size_t m_Size = 0;
			Type m_Type = None;
			wxCheckBoxState m_CheckState = wxCHK_UNDETERMINED;

		public:
			ComponentsDisplayModelNode()
			{
			}
			ComponentsDisplayModelNode(const KPPCGroup& group)
				:m_Group(&group), m_Type(Group)
			{
			}
			ComponentsDisplayModelNode(const KPPCEntry& entry)
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
				return IsGroup() ? m_Group : nullptr;
			}
			const KPPCEntry* GetEntry() const
			{
				return IsEntry() ? m_Entry : nullptr;
			}

			bool HasParentNode() const
			{
				return m_ParentNode != nullptr;
			}
			const ComponentsDisplayModelNode* GetParentNode() const
			{
				return m_ParentNode;
			}
			void SetParentNode(const ComponentsDisplayModelNode& node)
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
			void SetBounds(size_t beginIndex, size_t size)
			{
				m_Begin = beginIndex;
				m_Size = size;
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
}
