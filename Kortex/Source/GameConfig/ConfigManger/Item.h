#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"
#include "ItemValue.h"
#include "ItemOptions.h"
#include "ItemSamples.h"
#include "IViewItem.h"
#include "ISource.h"
#include "IAction.h"
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
			bool HasHash() const
			{
				return m_Hash.has_value();
			}
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

		private:
			ItemGroup& m_Group;
			wxString m_Category;
			wxString m_Path;
			wxString m_Name;
			wxString m_Label;
			ItemSamples m_Samples;
			ItemOptions m_Options;
			HashStore m_HashStore;
			TypeID m_TypeID;
			ActionValue m_Action;

			bool m_HasChanges = false;
			mutable std::optional<wxString> m_DisplayPath;

		protected:
			virtual void Clear() = 0;
			virtual void Read(const ISource& source) = 0;
			virtual void Write(ISource& source) const = 0;
			virtual void ChangeNotify();

			void RegisterAsKnown();
			void UnregisterAsKnown();

		public:
			Item(ItemGroup& group, const KxXMLNode& itemNode = {});
			virtual ~Item();

		public:
			virtual bool Create(const KxXMLNode& itemNode = {}) = 0;
			virtual bool IsOK() const;
			virtual bool IsUnknown() const = 0;
			virtual wxString GetFullPath() const;
			size_t GetHash() const
			{
				return m_HashStore.Get(*this);
			}
			
			bool HasChanges() const
			{
				return m_HasChanges;
			}
			void SaveChanges();
			void DiscardChanges();
			void DeleteValue();

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

			bool HasAction() const
			{
				return !m_Action.IsDefault();
			}
			ActionValue GetAction() const
			{
				return m_Action;
			}

			virtual bool IsEditable() const;

		public:
			void OnActivate(KxDataView2::Column& column) override;
			wxString GetViewString(ColumnID id) const override;

			wxAny GetValue(const KxDataView2::Column& column) const override;
			bool Compare(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
	};
}
