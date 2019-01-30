#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"
#include "ItemValue.h"
#include "ItemOptions.h"
#include "Application/RTTI.h"

namespace Kortex
{
	class IConfigManager;
}

namespace Kortex::GameConfig
{
	class ItemGroup;
	class Definition;
	class ISource;

	class Item: public RTTI::IInterface<Item>
	{
		friend class ItemGroup;

		private:
			ItemGroup& m_Group;
			wxString m_Category;
			wxString m_Path;
			wxString m_Name;
			wxString m_Label;
			TypeID m_TypeID;
			ItemKindValue m_Kind;
			ItemOptions m_Options;

		protected:
			virtual bool Create(const KxXMLNode& itemNode) = 0;
			virtual void Load(const ISource& source) = 0;
			virtual void Save(ISource& source) const = 0;

		public:
			Item(ItemGroup& group, const KxXMLNode& itemNode = KxXMLNode());
			virtual ~Item() = default;

		public:
			virtual bool IsOK() const;

			IConfigManager& GetManager() const;
			Definition& GetDefinition() const;
			ItemGroup& GetGroup() const
			{
				return m_Group;
			}
			
			wxString GetCategory() const
			{
				return m_Category;
			}
			void SetCategory(const wxString& category)
			{
				m_Category = category;
			}
			
			wxString GetPath() const
			{
				return m_Path;
			}
			void SetPath(const wxString& path)
			{
				m_Path = path;
			}

			wxString GetName() const
			{
				return m_Name;
			}
			void SetName(const wxString& name)
			{
				m_Name = name;
			}
			
			wxString GetLabel() const
			{
				return m_Label.IsEmpty() ? m_Name : m_Label;
			}
			void SetLabel(const wxString& label)
			{
				m_Label = label;
			}

			const ItemOptions& GetOptions() const
			{
				return m_Options;
			}
			void SetOptions(const ItemOptions& options)
			{
				m_Options = options;
			}

			ItemKindValue GetKind() const
			{
				return m_Kind;
			}
			void SetKind(ItemKindValue value)
			{
				m_Kind = value;
			}

			TypeID GetTypeID() const
			{
				return m_TypeID;
			}
			void SetTypeID(TypeID id)
			{
				m_TypeID = id;
			}
			DataType GetDataType() const;
	};
}
