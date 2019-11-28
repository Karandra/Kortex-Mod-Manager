#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageCreator/PageComponents/ComponentsModelNode.h"
#include "PackageProject/ModPackageProject.h"
#include "PackageProject/ComponentsSection.h"
#include <KxFramework/KxDataView.h>
class KxImageView;

namespace Kortex::PackageDesigner
{
	class WorkspaceDocument;
}
namespace Kortex::PackageProject
{
	class InterfaceSection;
	class ComponentsSection;
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ComponentsModel:
		public KxDataViewModelExBase<KxDataViewModel>,
		public KxDataViewModelExDragDropEnabled<ListModelDataObject>
	{
		public:
			using EntryID = ComponentsModelNode::EntryID;
			using AllItemsFunc = void(ComponentsModel::*)(ComponentsModelNode*, const wxString&);
	
		private:
			ComponentsModelNode::Vector m_Steps;
			WorkspaceDocument* m_Controller = nullptr;
	
			KxDataViewComboBoxEditor m_TypeDescriptorEditor;
			KxDataViewComboBoxEditor m_SelectionModeEditor;
			KxDataViewComboBoxEditor m_ImagesEditor;
			KxDataViewTextEditor m_TextEditor;
			KxImageView* m_EntryImageView = nullptr;
	
		protected:
			void OnInitControl() override;
			
			bool IsContainer(const KxDataViewItem& item) const override;
			void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			KxDataViewItem GetParent(const KxDataViewItem& item) const override;
			bool HasContainerColumns(const KxDataViewItem& item) const override;
	
			bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) override;
			bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
	
			void GetStepValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentStep* step, bool editor = false) const;
			void GetGroupValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentGroup* group, bool editor = false) const;
			void GetEntryValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentItem* entry, bool editor = false) const;
			void GetEntryItemValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentItem* entry, EntryID id, bool editor = false) const;
	
			bool SetStepValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentStep* step);
			bool SetGroupValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentGroup* group);
			bool SetEntryValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentItem* entry);
			bool SetEntryItemValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentItem* entry, EntryID id);
	
		private:
			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAttachEditor(KxDataViewEvent& event);
			void OnDetachEditor(KxDataViewEvent& event);
			void UpdateImageEditorList();
	
			KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			bool OnDragItems(KxDataViewEventDND& event) override;
			bool OnDropItems(KxDataViewEventDND& event) override;
			bool OnDropItemsPossible(KxDataViewEventDND& event) override;
	
			void AddStep(ComponentsModelNode* node, const KxDataViewItem& item);
			void AddGroup(ComponentsModelNode* node, const KxDataViewItem& item);
			void AddEntry(ComponentsModelNode* node, KxDataViewItem& item);
			void AddEntriesFromFiles(ComponentsModelNode* node, KxDataViewItem& item);
			void RemoveStep(ComponentsModelNode* node, const PackageProject::ComponentStep* step);
			void RemoveGroup(ComponentsModelNode* node, const PackageProject::ComponentGroup* group);
			void RemoveEntry(ComponentsModelNode* node, const PackageProject::ComponentItem* entry);
	
			KxMenu* CreateAllItemsMenu();
			void CreateAllItemsMenuEntry(KxMenu* menu, ComponentsModelNode* node, const wxString& name, AllItemsFunc func);
	
			void AllSteps_Name(ComponentsModelNode* node, const wxString& name);
			void AllSteps_Conditions(ComponentsModelNode* node, const wxString& name);
	
			void AllGroups_Name(ComponentsModelNode* node, const wxString& name);
			void AllGroups_SelectionMode(ComponentsModelNode* node, const wxString& name);
	
			void AllEntries_Name(ComponentsModelNode* node, const wxString& name);
			void AllEntries_DefaultTypeDescriptor(ComponentsModelNode* node, const wxString& name);
			void AllEntries_FileData(ComponentsModelNode* node, const wxString& name);
			void AllEntries_Requirements(ComponentsModelNode* node, const wxString& name);
			void AllEntries_Image(ComponentsModelNode* node, const wxString& name);
			void AllEntries_Conditions(ComponentsModelNode* node, const wxString& name);
			void AllEntries_AssignedFlags(ComponentsModelNode* node, const wxString& name);
			void AllEntries_Description(ComponentsModelNode* node, const wxString& name);
	
			PackageProject::InterfaceSection& GetInterface() const;
			PackageProject::ComponentsSection& GetComponents() const;
	
		public:
			ComponentsModel(WorkspaceDocument* controller);
	
		public:
			KxDataViewItem GetItem(const ComponentsModelNode* node) const
			{
				return KxDataViewItem(node);
			}
			ComponentsModelNode* GetNode(const KxDataViewItem& item) const
			{
				return item.GetValuePtr<ComponentsModelNode>();
			}
	
			void ChangeNotify();
			void NotifyChangedItem(const KxDataViewItem& item);
			void SetEntryImageView(KxImageView* pEntryImage)
			{
				m_EntryImageView = pEntryImage;
			}
			
			void RefreshItems() override;
			void SetProject(ModPackageProject& project);
	};
}
