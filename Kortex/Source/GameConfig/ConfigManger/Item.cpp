#include "stdafx.h"
#include "Item.h"
#include "ItemGroup.h"
#include "Definition.h"
#include "GameConfig/IConfigManager.h"
#include "GameConfig/ConfigManger/DisplayModel.h"
#include "Application/IApplication.h"
#include <KxFramework/DataView2/DataView2.h>
#include <KxFramework/KxStringUtility.h>
#include <KxFramework/KxComparator.h>

namespace Kortex::GameConfig
{
	size_t HashStore::Get(const Item& item, const wxString& value) const
	{
		if (!m_Hash)
		{
			wxString hashData = item.GetPath();

			// Add name
			wxString name = item.GetName();
			if (!name.IsEmpty())
			{
				hashData += wxS('/');
				hashData += name;
			}

			if (!value.IsEmpty())
			{
				hashData += wxS('/');
				hashData += value;
			}

			size_t hashValue = std::hash<wxString>()(hashData);
			if (hashValue != 0)
			{
				m_Hash = hashValue;
			}
		}
		return *m_Hash;
	}
}

namespace Kortex::GameConfig
{
	void Item::ChangeNotify()
	{
		m_HasChanges = true;
		GetManager().OnItemChanged(*this);
	}
	void Item::RegisterAsKnown()
	{
		if (!IsUnknown())
		{
			m_Group.AddKnownItem(GetHash(), *this);
		}
	}
	void Item::UnregisterAsKnown()
	{
		if (m_HashStore.HasHash())
		{
			m_Group.RemoveKnownItem(GetHash());
		}
	}

	Item::Item(ItemGroup& group, const KxXMLNode& itemNode)
		:m_Group(group), m_Samples(*this)
	{
		if (itemNode.IsOK())
		{
			// Main data
			m_Category = itemNode.GetAttribute(wxS("Category"));
			m_Path = itemNode.GetAttribute(wxS("Path"));
			m_Name = itemNode.GetAttribute(wxS("Name"));
			m_TypeID.FromString(itemNode.GetAttribute(wxS("Type")));

			// Label
			m_Label = GetManager().TranslateItemLabel(itemNode.GetFirstChildElement(wxS("Label")), m_Name, wxS("ValueName"), false);
			if (m_Label.IsEmpty())
			{
				m_Label = GetManager().TranslateItemLabel(m_Path.AfterLast(wxS('/')), wxS("ValueName"));
			}

			// Options
			m_Options.Load(itemNode.GetFirstChildElement(wxS("Options")), GetDataType());
			m_Options.CopyIfNotSpecified(group.GetOptions(), GetDataType());

			// Samples
			m_Samples.Load(itemNode.GetFirstChildElement(wxS("Samples")));

			// Action
			m_Action.FromString(itemNode.GetFirstChildElement(wxS("Action")).GetAttribute(wxS("Name")));
		}
	}
	Item::~Item()
	{
		UnregisterAsKnown();
		DetachAllChildren();
	}

	bool Item::IsOK() const
	{
		return !m_Category.IsEmpty() && !m_Path.IsEmpty() && m_TypeID.IsDefinitiveType();
	}
	wxString Item::GetFullPath() const
	{
		wxString fullPath;
		auto AddPart = [&fullPath](const wxString& part)
		{
			if (!fullPath.IsEmpty() && !part.IsEmpty())
			{
				fullPath += wxS('/');
			}
			if (!part.IsEmpty())
			{
				fullPath += part;
			}
		};

		AddPart(m_Group.GetSource().GetPathDescription());
		AddPart(m_Path);
		AddPart(m_Name);

		return fullPath;
	}
	wxString Item::GetViewString(ColumnID id) const
	{
		switch (id)
		{
			case ColumnID::Path:
			{
				if (!m_DisplayPath)
				{
					m_DisplayPath = GetFullPath();
				}
				return *m_DisplayPath;
			}
			case ColumnID::Name:
			{
				return m_Label;
			}
			case ColumnID::Type:
			{
				return m_TypeID.ToString();
			}
		};
		return {};
	}

	IConfigManager& Item::GetManager() const
	{
		return m_Group.GetManager();
	}
	Definition& Item::GetDefinition() const
	{
		return m_Group.GetDefinition();
	}
	wxWindow* Item::GetInvokingWindow() const
	{
		if (KxDataView2::View* view = GetView())
		{
			return view;
		}
		else if (DisplayModel* displayModel = GetManager().GetDisplayModel())
		{
			return displayModel->GetView();
		}
		return nullptr;
	}
	wxWindow* Item::GetInvokingTopLevelWindow() const
	{
		if (wxWindow* window = GetInvokingWindow())
		{
			if (wxWindow* topLevel = wxGetTopLevelParent(window))
			{
				return topLevel;
			}
		}
		return IApplication::GetInstance()->GetTopWindow();
	}

	void Item::SaveChanges()
	{
		if (m_HasChanges)
		{
			Write(m_Group.GetSource());
			m_HasChanges = false;
		}
	}
	void Item::DiscardChanges()
	{
		if (m_HasChanges)
		{
			Clear();
			Read(m_Group.GetSource());
			m_HasChanges = false;
		}
	}
	void Item::DeleteValue()
	{
		Clear();
		ChangeNotify();
	}

	DataType Item::GetDataType() const
	{
		return m_Group.GetDefinition().GetDataType(m_TypeID);
	}
	bool Item::IsEditable() const
	{
		switch (m_Options.GetEditableBehavior().GetValue())
		{
			case EditableBehaviorID::Editable:
			{
				return true;
			}
			case EditableBehaviorID::Selectable:
			case EditableBehaviorID::Inert:
			{
				return false;
			}
			case EditableBehaviorID::Auto:
			{
				return IsUnknown() || m_TypeID.IsFloat() || (m_TypeID.IsString() || m_TypeID.IsInteger() && m_Samples.IsEmpty());
			}
			case EditableBehaviorID::EditableIfNoSamples:
			{
				return m_Samples.IsEmpty();
			}
		};
		return false;
	}

	wxAny Item::GetValue(const KxDataView2::Column& column) const
	{
		const ColumnID id = column.GetID<ColumnID>();
		if (id == ColumnID::Path)
		{
			if (m_HasChanges)
			{
				return KxDataView2::BitmapTextValue(GetViewString(id), KGetBitmap(KIMG_PENCIL_SMALL));
			}
			else
			{
				return GetViewString(id);
			}
		}
		return GetViewString(id);
	}
	bool Item::Compare(const KxDataView2::Node& node, const KxDataView2::Column& column) const
	{
		const IViewItem* viewItem = nullptr;
		if (node.QueryInterface(viewItem))
		{
			const ColumnID id = column.GetID<ColumnID>();
			return KxComparator::IsLess(GetViewString(id), viewItem->GetViewString(id), true);
		}
		return false;
	}

	void Item::OnActivate(KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Path)
		{
			ToggleExpanded();
		}
	}
}
