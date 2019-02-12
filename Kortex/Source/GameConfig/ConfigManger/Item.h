#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"
#include "ItemValue.h"
#include "ItemOptions.h"
#include "ItemSamples.h"
#include "IViewItem.h"
#include "ISource.h"
#include "GameConfig/IConfigManager.h"
#include <KxFramework/DataView2/Node.h>
#include <KxFramework/DataView2/DataView2Fwd.h>

namespace Kortex
{
	class IConfigManager;
}

namespace Kortex::GameConfig
{
	class Item;

	class HashStore
	{
		private:
			mutable std::optional<size_t> m_Hash;

		public:
			size_t Get(const Item& item, const wxString& value = {}) const;
	};
}

namespace Kortex::GameConfig
{
	class ItemGroup;
	class Definition;

	class Item: public RTTI::IExtendInterface<Item, IViewItem, KxDataView2::Node>
	{
		friend class ItemGroup;
		friend class HashStore;

		private:
			ItemGroup& m_Group;
			wxString m_Category;
			wxString m_Path;
			wxString m_Name;
			wxString m_Label;
			ItemSamples m_Samples;
			TypeID m_TypeID;
			ItemOptions m_Options;

			bool m_HasChanges = false;
			mutable std::optional<wxString> m_DisplayPath;

		protected:
			virtual bool Create(const KxXMLNode& itemNode) = 0;
			virtual void Clear() = 0;
			virtual void Read(const ISource& source) = 0;
			virtual void Write(ISource& source) const = 0;

			size_t CalcHash(const wxString& value = {}) const;
			virtual void ChangeNotify();

		public:
			Item(ItemGroup& group, const KxXMLNode& itemNode = {});
			virtual ~Item();

		public:
			virtual bool IsOK() const;
			virtual bool IsUnknown() const = 0;
			virtual size_t GetHash() const = 0;
			virtual wxString GetFullPath() const;
			wxString GetStringRepresentation(ColumnID id) const override;
			
			bool HasChanges() const
			{
				return m_HasChanges;
			}
			void SaveChanges();
			void DiscardChanges();

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
			
			virtual wxString GetPath() const
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

			bool HasSamples() const
			{
				return !m_Samples.IsEmpty();
			}
			const ItemSamples& GetSamples() const
			{
				return m_Samples;
			}
			ItemSamples& GetSamples()
			{
				return m_Samples;
			}

			const ItemOptions& GetOptions() const
			{
				return m_Options;
			}
			ItemOptions& GetOptions()
			{
				return m_Options;
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

			virtual bool IsEditable() const;

		public:
			virtual void OnActivate(KxDataView2::Column& column) {}
			virtual void OnSelect(KxDataView2::Column& column) {}

			wxAny GetValue(const KxDataView2::Column& column) const override;
			bool Compare(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
	};
}
