#pragma once
#include "stdafx.h"
#include "../Common.h"

namespace Kortex::InstallWizard::ComponentsPageNS
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
			union
			{
				const PackageProject::ComponentGroup* m_Group = nullptr;
				const PackageProject::ComponentItem* m_Entry;
			};
			const DisplayModelNode* m_ParentNode = nullptr;

			size_t m_Begin = 0;
			size_t m_Size = 0;
			Type m_Type = None;
			wxCheckBoxState m_CheckState = wxCHK_UNDETERMINED;

		public:
			DisplayModelNode() = default;
			DisplayModelNode(const PackageProject::ComponentGroup& group)
				:m_Group(&group), m_Type(Group)
			{
			}
			DisplayModelNode(const PackageProject::ComponentItem& entry)
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

			const PackageProject::ComponentGroup* GetGroup() const
			{
				return IsGroup() ? m_Group : nullptr;
			}
			const PackageProject::ComponentItem* GetEntry() const
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
				if (const PackageProject::ComponentItem* entry = GetEntry())
				{
					return entry->GetTDCurrentValue() == PackageProject::TypeDescriptor::Required;
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
